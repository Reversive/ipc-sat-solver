#include "include/master.h"

int master_fd[PIPE_ARRAY_SIZE];
int slave_fd[PIPE_ARRAY_SIZE];
int slave_pid[MAX_SLAVE_COUNT];
const int DELIM = '|';

file_container queue = {{0}, 0, 0, 0};

int main(
    int argc, 
    char **argv) 
{
    setbuf(stdout, NULL);
    int file_count = argc;
    char **file_names = argv;
    int slave_count = file_count >= MAX_SLAVE_COUNT ? MAX_SLAVE_COUNT : file_count;
    open_pipe_array(master_fd, slave_count);
    open_pipe_array(slave_fd, slave_count);
    distribute_files(master_fd, file_names, file_count, slave_count);
    summon_slaves(slave_count);
    //sleep(VIEW_SLEEP_INTERVAL); // time for view to spawn
    slave_container sc[slave_count];
    init_container_array(sc, slave_count);
    int status = RUNNING;

    while(status == RUNNING)
    {
        fd_set rfds;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        int fdmax = set_fd_array(slave_fd, &rfds, slave_count);
        int retval = select(fdmax, &rfds, NULL, NULL, &tv);
        switch (retval)
        {
        case SYS_FAILURE:
            perror("select()");
            exit(SELECT_FAILURE);

        case TIMEOUT:
            if(queue.queue_read == queue.queue_size)
            {
                status = STOP;
            }
            break;
        
        default:
            for(int i = 0; i < slave_count; i++)
            {
                int fdr = slave_fd[i * PIPE_SIZE];
                int fdw = master_fd[i * PIPE_SIZE + OUT];
                if(FD_ISSET(fdr, &rfds))
                {
                    int bytes_read = read(fdr, sc[i].buffer + sc[i].pos , CHUNK_SIZE);
                    if(bytes_read == SYS_FAILURE)
                    {
                        perror("read()");
                        exit(EXIT_FAILURE);
                    }
                    else if(bytes_read == CHUNK_SIZE)
                    {
                        char *offset_ptr = strchr(sc[i].buffer + sc[i].pos, DELIM);
                        if( offset_ptr != NULL)
                        {
                            int size = offset_ptr - sc[i].buffer;
                            FILE *fp = fopen ("result.txt", "a"); //to-do: check open == null
                            fwrite(sc[i].buffer, sizeof(char), size, fp);
                            fwrite("\n\n", sizeof(char), 2, fp);
                            fclose(fp);
                            fix_internal_buffer(i, sc, offset_ptr);
                            if(queue.queue_pos < queue.queue_size)
                            {
                                queue_next_file(fdw);
                            }

                        }
                        else
                        {
                            sc[i].pos += CHUNK_SIZE;
                        }
                    }
                    else if(bytes_read < CHUNK_SIZE)
                    {
                        FILE *fp = fopen ("result.txt", "a");                       
                        int size = sc[i].pos + bytes_read - 1;
                        fwrite(sc[i].buffer, sizeof(char), size, fp);
                        fwrite("\n\n", sizeof(char), 2, fp);
                        fclose(fp);
                        reset_container(sc, i);
                        if(queue.queue_pos < queue.queue_size)
                        {
                            queue_next_file(fdw);
                        }
                    }
                }
                
            }
            break;
        }
       
    }

    close_pipe(master_fd, OUT, slave_count); // send signal to slaves to exit
    close_pipe(master_fd, IN, slave_count);
    close_pipe(slave_fd, OUT, slave_count);
    close_pipe(slave_fd, IN, slave_count);

    for(int i = 0; i < slave_count; i++)
    {
        int fr;
        waitpid(slave_pid[i], &fr, 0);
    }

}


void fix_internal_buffer(
    int idx, 
    slave_container * sc, 
    char * delim_offset)
{
    char * chunk_pos = sc[idx].buffer + sc[idx].pos;
    int new_file_pos = CHUNK_SIZE -  (delim_offset - chunk_pos) ;
    char tmp[new_file_pos];
    memcpy(tmp, chunk_pos + CHUNK_SIZE - new_file_pos, new_file_pos);
    reset_container(sc, idx);
    memcpy(sc[idx].buffer, tmp, new_file_pos);
    sc[idx].pos = new_file_pos;
}


void queue_next_file(int fd)
{
    int sz = strlen(queue.file_buffer[queue.queue_pos]) + NULL_TERMINATOR;
    write_fd(fd, queue.file_buffer[queue.queue_pos++], sz);
    write_fd(fd, "\n", BYTE);
    queue.queue_read++;
}

void distribute_files(
    int * fda, 
    char ** file_list, 
    int file_count,
    int slave_count)
{
    int i;
    for(i = 0; i < file_count && i != STARTING_FILE_LIMIT; i++)
    {
        write_pipe(fda, i % slave_count, file_list[i], strlen(file_list[i]) + NULL_TERMINATOR);
        write_pipe(fda, i % slave_count, "\n", BYTE);
    }
    for(int j = i; j < file_count; j++)
    {
        queue.file_buffer[queue.queue_size++] = file_list[j];
    }
}

void summon_slaves(
    int slave_count)
{
    pid_t pid;
    char *argv[] = { NULL };
    for(int i = 0; i < slave_count; i++)
    {
        pid = fork();

        if(pid == SYS_FAILURE)
        {
            perror("fork()");
            kill_previous_slaves(i);
            exit(SLAVE_FORK_FAILURE);
        }

        if(pid == 0)
        {
            redirect_pipe(master_fd, i, IN, STDIN, slave_count);
            close_pipe(master_fd, IN, slave_count);
            close_pipe(master_fd, OUT, slave_count);
            redirect_pipe(slave_fd, i, OUT, STDOUT, slave_count);
            close_pipe(slave_fd, IN, slave_count);
            close_pipe(slave_fd, OUT, slave_count);  
            execv("./process/bin/slave", argv);
            perror("execv()");
            exit(EXECV_FAILURE);
        }

        slave_pid[i] = pid;
    }
}


void kill_previous_slaves(int last_slave)
{
    int i;
    for(i = 0; i < last_slave; i++)
    {
        kill(slave_pid[i], KILL_SIGNAL);
    }
}

