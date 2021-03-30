#include "include/master.h"

int master_fd[PIPE_ARRAY_SIZE];
int slave_fd[PIPE_ARRAY_SIZE];
int slave_pid[MAX_SLAVE_COUNT];
const int DELIM = '*';

file_container queue = {{0}, 0, 0, 0};

int main(
    int argc, 
    char **argv) 
{
    sleep(VIEW_SLEEP_INTERVAL);
    setbuf(stdout, NULL);
    int path_count = argc;
    char **paths = argv;
    int slave_count = path_count >= MAX_SLAVE_COUNT ? MAX_SLAVE_COUNT : path_count;
    //int shm_size = path_count * MAX_BUFFER_SIZE;
    //int shm_object = open_shared_mem_object(NAME, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    //extend_shared_mem(shm_object, shm_size);
    //void *shm_data = map_shared_memory(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_object, 0);
    //unlink_shared_memory(NAME);
    open_pipe_array(master_fd, slave_count);
    open_pipe_array(slave_fd, slave_count);
    distribute_and_cache_paths(master_fd, paths, path_count, slave_count);
    summon_slaves(slave_count);
    slave_container sc[slave_count];
    init_container_array(sc, slave_count);
    int slaves_status[slave_count];
    set_slaves_status(slaves_status, slave_count, RUNNING);
    int slaves_running = get_running_slaves_count(slaves_status, slave_count);
    int is_pipe_closed = 0;

    while(slaves_running > 0)
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        int fdmax = set_pipe_array(slave_fd, &rfds, slave_count);
        int retval = select(fdmax, &rfds, NULL, NULL, NULL);
        if(retval == SYS_FAILURE)
        {
            perror("select()");
            exit(SELECT_FAILURE);
        }
        else if(retval > 0)
        {
            for(int current_slave = 0; current_slave < slave_count; current_slave++)
            {
                int read_fd = slave_fd[current_slave * PIPE_SIZE];
                int write_fd = master_fd[current_slave * PIPE_SIZE + OUT];
                if(FD_ISSET(read_fd, &rfds))
                {
                    int bytes_read = read(read_fd, sc[current_slave].buffer + sc[current_slave].pos , CHUNK_SIZE);
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
                        char *delim_pos = strchr(sc[current_slave].buffer, DELIM);
                        if(delim_pos)
                        {
                            int size = delim_pos - sc[current_slave].buffer - DELIMITER;
                            write_buffer_to_file("result.txt", "a", size, sc, current_slave);                          
                            fix_internal_buffer(current_slave, sc, delim_pos);
                            poll_queue_next_path(&is_pipe_closed, write_fd, slave_count);
                        }
                        else
                        {
                            sc[current_slave].pos += CHUNK_SIZE;
                        }
                    }
                    else if(bytes_read < CHUNK_SIZE)
                    {                 
                        int size = sc[current_slave].pos + bytes_read - DELIMITER;
                        write_buffer_to_file("result.txt", "a", size, sc, current_slave);
                        reset_container(sc, current_slave);
                        poll_queue_next_path(&is_pipe_closed, write_fd, slave_count);
                    }
                }
                
            }
            slaves_running = get_running_slaves_count(slaves_status, slave_count);
        }
       
    }

    close_pipe(slave_fd, IN, slave_count);
    for(int current_slave = 0; current_slave < slave_count; current_slave++)
    {
        int fr;
        waitpid(slave_pid[current_slave], &fr, 0);
    }

}


void poll_queue_next_path(
    int * is_pipe_closed,
    int write_fd,
    int slave_count)
{
    if(queue.queue_pos < queue.queue_size)
    {
        queue_next_path(write_fd);
    }
    else if(!*is_pipe_closed)
    {
        *is_pipe_closed = !*is_pipe_closed;
        close_pipe(master_fd, OUT, slave_count);
    }
}

void queue_next_path(
    int fd)
{
    int sz = strlen(queue.file_buffer[queue.queue_pos]) + NULL_TERMINATOR;
    write_fd(fd, queue.file_buffer[queue.queue_pos++], sz);
    write_fd(fd, "\n", BYTE);
    queue.queue_read++;
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
    char tmp[new_file_pos - DELIMITER];
    memcpy(tmp, chunk_pos + CHUNK_SIZE - new_file_pos + DELIMITER, new_file_pos - DELIMITER);
    reset_container(sc, idx);
    memcpy(sc[idx].buffer, tmp, new_file_pos - DELIMITER);
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
        write_pipe(pipe, current_path % slave_count, path_list[current_path], strlen(path_list[current_path]) + NULL_TERMINATOR);
        write_pipe(pipe, current_path % slave_count, "\n", BYTE);
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
            redirect_pipe(master_fd, current_slave, IN, STDIN, slave_count);
            close_pipe(master_fd, IN, slave_count);
            close_pipe(master_fd, OUT, slave_count);
            redirect_pipe(slave_fd, current_slave, OUT, STDOUT, slave_count);
            close_pipe(slave_fd, IN, slave_count);
            close_pipe(slave_fd, OUT, slave_count);  
            execv("./process/bin/slave", argv);
            perror("execv()");
            exit(EXECV_FAILURE);
        }

        slave_pid[current_slave] = pid;
    }
    close_pipe(master_fd, IN, slave_count);
    close_pipe(slave_fd, OUT, slave_count);
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