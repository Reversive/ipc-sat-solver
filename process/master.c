#include "include/master.h"

int master_to_slave_pipe_array[PIPE_ARRAY_SIZE];
int slave_to_master_pipe_array[PIPE_ARRAY_SIZE];
int slave_pid[MAX_SLAVE_COUNT];
const int DELIM = '*';

file_container queue = {{0}, 0, 0, 0};

int main(
    int argc, 
    char **argv) 
{
    setbuf(stdout, NULL);
    int path_count = argc;
    char **paths = argv;
    int slave_count = path_count >= MAX_SLAVE_COUNT ? MAX_SLAVE_COUNT : path_count;
    int shm_object = open_shared_mem_object(NAME, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    extend_shared_mem(shm_object, sizeof(shm_buffer));
    shm_buffer *shm_data = map_shared_memory(NULL, sizeof(*shm_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_object, 0);
    init_shared_mem_data(shm_data, 0);
    shm_data->path_count = path_count;
    init_semaphore(&shm_data->bouncer, PROCESS_SHARED, 0);
    sleep(VIEW_SLEEP_INTERVAL);
    open_pipe_array(master_to_slave_pipe_array, slave_count);
    open_pipe_array(slave_to_master_pipe_array, slave_count);
    distribute_and_cache_paths(master_to_slave_pipe_array, paths, path_count, slave_count);
    summon_slaves(slave_count);
    slave_container sc[slave_count];
    init_container_array(sc, slave_count);
    int slaves_status[slave_count];
    set_slaves_status(slaves_status, slave_count, RUNNING);
    int slaves_running = get_running_slaves_count(slaves_status, slave_count);
    int is_pipe_closed = 0;

    while(slaves_running > 0)
    {
        fd_set fd_reads;
        FD_ZERO(&fd_reads);
        int fdmax = set_pipe_array(slave_to_master_pipe_array, &fd_reads, slave_count);
        int retval = select(fdmax, &fd_reads, NULL, NULL, NULL);
        if(retval == SYS_FAILURE)
        {
            perror("select()");
            exit(SELECT_FAILURE);
        }
        else if(retval > 0)
        {
            for(int current_slave = 0; current_slave < slave_count; current_slave++)
            {
                int fd_read = slave_to_master_pipe_array[current_slave * PIPE_SIZE + READ];
                int fd_write = master_to_slave_pipe_array[current_slave * PIPE_SIZE + WRITE];
                if(FD_ISSET(fd_read, &fd_reads))
                {
                    int bytes_read = read(fd_read, sc[current_slave].buffer + sc[current_slave].pos , CHUNK_SIZE);
                    if(bytes_read == SYS_FAILURE)
                    {
                        perror("read()");
                        exit(EXIT_FAILURE);
                    }
                    else if(bytes_read == NONE)
                    {
                        slaves_status[current_slave] = STOP;
                    }
                    else if(bytes_read == CHUNK_SIZE)
                    {
                        char *delimiter_offset = strchr(sc[current_slave].buffer, DELIM);
                        if(delimiter_offset != NULL)
                        {
                            int size = delimiter_offset - sc[current_slave].buffer;
                            write_buffer_to_file("result.txt", "a", size, sc, current_slave);
                            write_buffer_to_shared_memory(shm_data, size, sc[current_slave].buffer);        
                            fix_internal_buffer(current_slave, sc, delimiter_offset);
                            poll_queue_next_path(&is_pipe_closed, fd_write, slave_count);
                        }
                        else
                        {
                            sc[current_slave].pos += CHUNK_SIZE;
                        }
                    }
                    else if(bytes_read < CHUNK_SIZE)
                    {
                        
                        int size = sc[current_slave].pos + bytes_read - DELIMITER;
                        write_buffer_to_shared_memory(shm_data, size, sc[current_slave].buffer);
                        write_buffer_to_file("result.txt", "a", size, sc, current_slave);
                        reset_container(sc, current_slave);
                        poll_queue_next_path(&is_pipe_closed, fd_write, slave_count);
                    }
                }
                
            }
            slaves_running = get_running_slaves_count(slaves_status, slave_count);
        }
       
    }

    close_pipe_array(slave_to_master_pipe_array, READ, slave_count);
    for(int current_slave = 0; current_slave < slave_count; current_slave++)
    {
        int fr;
        waitpid(slave_pid[current_slave], &fr, 0);
    }
    unmap_shared_memory(shm_data, sizeof(*shm_data));
    unlink_shared_memory(NAME);
    // close(shm_data);
    // return 0;
}


void write_buffer_to_shared_memory(
    shm_buffer * dest, 
    int size, 
    char * buffer)
{
    dest->size[dest->write_position] = size;
    memcpy(dest->buffer[dest->write_position++], buffer, size);
    post_semaphore(&dest->bouncer);
}

void poll_queue_next_path(
    int * is_pipe_closed,
    int fd_write,
    int slave_count)
{
    if(queue.queue_pos < queue.queue_size)
    {
        queue_next_path(fd_write, queue.file_buffer, queue.queue_pos);
        queue.queue_pos++;
    }
    else if(!*is_pipe_closed)
    {
        *is_pipe_closed = !*is_pipe_closed;
        close_pipe_array(master_to_slave_pipe_array, WRITE, slave_count);
    }
}

void queue_next_path(
    int fd,
    char **path_list,
    int current_file)
{
    int sz = strlen(path_list[current_file]) + NULL_TERMINATOR;
    char holder[sz + DELIMITER];
    memcpy(holder, path_list[current_file], sz);
    strcat(holder, "\n");
    write_fd(fd, holder, sz);    
}

void write_buffer_to_file(
    char * path,
    char * flag,
    int size,
    slave_container * sc,
    int current_slave)
{
    FILE *fp = fopen (path, flag);
    if(fp == NULL)
    {
        perror("Failed opening a file");
        exit(EXIT_FAILURE);
    }
    fwrite(sc[current_slave].buffer, sizeof(char), size, fp);
    fwrite("\n\n", sizeof(char), 2, fp);
    if( ferror( fp ) != 0 )
    {
        perror("Unexpected problem writing file");
        exit(EXIT_FAILURE);
    }
    fclose(fp);
}

void fix_internal_buffer(
    int idx, 
    slave_container * sc, 
    char * delim_offset)
{
    char * chunk_pos = sc[idx].buffer + sc[idx].pos;
    int new_file_pos = CHUNK_SIZE -  (delim_offset - chunk_pos) ;
    char tmp[new_file_pos];
    memcpy(tmp, chunk_pos + CHUNK_SIZE - new_file_pos + DELIMITER, new_file_pos);
    reset_container(sc, idx);
    memcpy(sc[idx].buffer, tmp, new_file_pos);
    sc[idx].pos = new_file_pos - DELIMITER;
}


void distribute_and_cache_paths(
    int * pipe, 
    char ** path_list, 
    int path_count,
    int slave_count)
{
    int current_path;
    for(current_path = 0; current_path < path_count && current_path != STARTING_FILE_LIMIT; current_path++)
    {
        int fd = pipe[PIPE_SIZE * (current_path % slave_count) + WRITE];
        queue_next_path(fd, path_list, current_path);
    }

    for(int j = current_path; j < path_count; j++)
    {
        queue.file_buffer[queue.queue_size++] = path_list[j];
    }
}

void summon_slaves(
    int slave_count)
{
    pid_t pid;
    char *argv[] = { NULL };
    for(int current_slave = 0; current_slave < slave_count; current_slave++)
    {
        pid = fork();

        if(pid == SYS_FAILURE)
        {
            perror("fork()");
            kill_previous_slaves(current_slave);
            exit(SLAVE_FORK_FAILURE);
        }

        if(pid == 0)
        {
            redirect_pipe(master_to_slave_pipe_array, current_slave, READ, STDIN, slave_count);
            close_pipe_array(master_to_slave_pipe_array, READ, slave_count);
            close_pipe_array(master_to_slave_pipe_array, WRITE, slave_count);
            redirect_pipe(slave_to_master_pipe_array, current_slave, WRITE, STDOUT, slave_count);
            close_pipe_array(slave_to_master_pipe_array, READ, slave_count);
            close_pipe_array(slave_to_master_pipe_array, WRITE, slave_count);  
            execv("./process/bin/slave", argv);
            perror("execv()");
            exit(EXECV_FAILURE);
        }

        slave_pid[current_slave] = pid;
    }
    close_pipe_array(master_to_slave_pipe_array, READ, slave_count);
    close_pipe_array(slave_to_master_pipe_array, WRITE, slave_count);
}


void kill_previous_slaves(int last_slave)
{
    int current_slave;
    for(current_slave = 0; current_slave < last_slave; current_slave++)
    {
        kill(slave_pid[current_slave], KILL_SIGNAL);
    }
}

void set_slaves_status(
    int * slave_status, 
    int slave_count,
    enum STATUS status)
{
    for(int current_slave = 0; current_slave < slave_count; current_slave++)
    {
        slave_status[current_slave] = status;
    }
}

int get_running_slaves_count(
    int * slave_status, 
    int slave_count)
{
    int running = 0;
    for(int current_slave = 0; current_slave < slave_count; current_slave++)
    {
        if(slave_status[current_slave] == RUNNING) running += 1;
    }
    return running;
}