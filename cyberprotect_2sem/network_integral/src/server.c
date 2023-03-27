#include <errno.h>
#include <network.h>

int sktcp_keepalive(int sk_tcp) 
{
    if (sk_tcp <= 0)
        return -1;
    
    // Enable keep alive option
    int enable = 1;
    check_error_return(setsockopt(sk_tcp, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable)));
    
    int idle = 3;
    check_error_return(setsockopt(sk_tcp, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle)));
    
    int interval = 1;
    check_error_return(setsockopt(sk_tcp, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)));
    
    int maxpkt = 3;
    check_error_return(setsockopt(sk_tcp, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(maxpkt)));

    return 0;
}

void destroy_node(node_t* node)
{
    if (!node)
        return;
    
    if (node->socket != 0)
        close(node->socket);
    
    deque_destroy(node->tasks);
}

static int get_udp_socket()
{
    int sk_br = socket(AF_INET, SOCK_DGRAM, 0);
    check_error_return(sk_br);

    // Set socket's options
    int enable = 1;
    check_error_return(setsockopt(sk_br, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)));
    check_error_return(setsockopt(sk_br, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)));

    return sk_br;
}

static int broadcast_server()
{
    // Initialize sockaddr structure for UDP connection
    struct sockaddr_in udp_addr;
    bzero(&udp_addr, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(PORT);
    udp_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    // Get configured UDP socket
    int sk_br = get_udp_socket();
    check_error_return(sk_br);

    // Send a message
    check_error_return(sendto(sk_br, BR_MSG, strlen(BR_MSG), 0, (struct sockaddr*) &udp_addr, sizeof(udp_addr)));

    return 0;
}

static int get_tcp_socket(struct sockaddr_in* tcp_addr)
{
    // Initialize TCP socket
    int sk_tcp = socket(AF_INET, SOCK_STREAM, 0);
    check_error_return(sk_tcp);

    int enable = 1;
    check_error_return(setsockopt(sk_tcp, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)));
    check_error_return(sktcp_keepalive(sk_tcp));

    check_error_return(bind(sk_tcp, (struct sockaddr*) tcp_addr, sizeof(*tcp_addr)));
    check_error_return(listen(sk_tcp, 8));

    // Set the listening socket to NON_BLOCK
    int flags = fcntl(sk_tcp, F_GETFL, 0);
    check_error_return(flags);
    check_error_return(fcntl(sk_tcp, F_SETFL, flags | O_NONBLOCK));

    return sk_tcp;
}

// returns sum of all threads avaliable on computers
static int accept_clients(int sk_tcp, struct sockaddr_in* tcp_addr, long ncomp, node_t* nodes)
{
    // Accept all computers and recv their socket information
    long naccepted = 0; // accepted computers
    long nthrecv = 0; // receive threads

    while (1)
    {
        if (naccepted == ncomp && nthrecv == ncomp)
            break;

        // Add the server socket and client's one each iteration
        fd_set fdset = {};
        FD_ZERO(&fdset);
        FD_SET(sk_tcp, &fdset);

        struct timeval timeval = {
            .tv_usec = 0,
            .tv_sec = 10
        };

        for (int i = 0; i < naccepted; ++i) 
        {
            // Add all accepted computers to the file set
            FD_SET(nodes[i].socket, &fdset);
        }

        int ret = select(FD_SETSIZE, &fdset, NULL, NULL, &timeval);
        check_error_return(ret);
        if (ret == 0)
            break; // all done
            
        if (FD_ISSET(sk_tcp, &fdset)) 
        {
            // Accept new connection
            socklen_t socklen = sizeof(*tcp_addr);
            nodes[naccepted].socket = accept(sk_tcp, (struct sockaddr*) tcp_addr, &socklen);
            check_error_return(nodes[naccepted].socket);

            nodes[naccepted].state = STATE_OK;

            // Send a message to the console
            printf("Accepted new node! Number: %ld\n", naccepted++);
        }

        for (int i = 0; i < naccepted; ++i) 
        {
            if (FD_ISSET(nodes[i].socket, &fdset)) 
            {
                // Receive thread info
                ret = recv(nodes[i].socket, &nodes[i].threads, sizeof(nodes[i].threads), 0);
                if (ret != sizeof(nodes[i].threads))
                    return -1;

                nthrecv++;
                printf("Node %d number of threads: %ld\n", i, nodes[i].threads);
            }
        }
    }

    if (naccepted != ncomp || nthrecv != ncomp) 
    {
        printf("Some computers didn't manage to connect!\n");
        return EXIT_FAILURE;
    }

    return 0;
}

static void print_task(const task_t* task, int i)
{
    printf("Node %d, task dump: a = %g, b = %g\n", i, task->a, task->b);
}

// count all nodes tasks and return this value
static int count_all_tasks(const node_t* nodes, long ncomp)
{
    int count = 0;
    for (long i = 0; i < ncomp; ++i)
        if (nodes[i].state == STATE_OK)
        {
            count += deque_size(nodes[i].tasks); // to receive
        }
    
    return count;
}

// send task. if it fails, mark node as timeout and wait for another redistribution
static int send_task(node_t* nodes, task_t* task, int i)
{
    if (send(nodes[i].socket, task, sizeof(*task), 0) != sizeof(*task))
    {
        printf("Failed to send new task to node %d\n", i);

        nodes[i].state = STATE_TIMEOUT;
        printf("Node %d has been marked as timeout\n", i);
        
        return -1;
    }

    return 0;
}

static int create_tasks(node_t* nodes, double a, double b, int ncomp)
{
    // count threads of alive nodes
    long sum = 0;
    for (int i = 0; i < ncomp; ++i)
        if (nodes[i].state == STATE_OK)
            sum += nodes[i].threads;

    if (sum == 0)
        return -1; // no alive threads

    double diff = (b - a) / sum;
    task_t prev_task = {};
    
    if (nodes[0].state == STATE_OK)
    {
        prev_task.a = a;
        prev_task.b = a + nodes[0].threads * diff;

        if (deque_push_back(nodes[0].tasks, prev_task) < 0)
            return -1;
        print_task(&prev_task, 0);

        send_task(nodes, &prev_task, 0);
    }
    else
    {
        prev_task.b = a;
    }
    
    for (int i = 1; i < ncomp; ++i)
    {
        if (nodes[i].state == STATE_OK)
        {
            task_t task = {
                .a = prev_task.b,
                .b = prev_task.b + nodes[i].threads * diff
            };

            prev_task.b = task.b;
            if (deque_push_back(nodes[i].tasks, task) < 0)
                return -1;
            print_task(&task, i);

            send_task(nodes, &task, i);
        }
    }
    
    return 0;
}

static int server_routine(long ncomp, long a, long b)
{
    int ret_code = 0;

    printf("Computers got: %ld\n", ncomp);

    node_t* nodes = (node_t*) malloc(sizeof(node_t) * ncomp);
    if (!nodes)
        return -ENOMEM;

    for (long i = 0; i < ncomp; ++i)
    {
        nodes[i].socket = 0;
        nodes[i].threads = 0;
        nodes[i].state = STATE_NONE;
        nodes[i].tasks = deque_get();
        if (!nodes[i].tasks)
            return -ENOMEM;
    }

    // Send broadcast message for cleints to discover server
    if (broadcast_server() < 0)
    {
        ret_code = -ENOMEM;
        goto exit_server_routine;
    }

    // Initialize sockaddr structure for TCP connection
    struct sockaddr_in tcp_addr;
    bzero(&tcp_addr, sizeof(tcp_addr));
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_port = htons(PORT);
    tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Get configured TCP socket
    int sk_tcp = get_tcp_socket(&tcp_addr);
    if (sk_tcp < 0)
    {
        ret_code = -ENOMEM;
        goto exit_server_routine;
    }

    // Accept clients, get all threads count
    int ret = accept_clients(sk_tcp, &tcp_addr, ncomp, nodes);
    if (ret < 0)
    {
        ret_code = -ENOMEM;
        goto exit_server_routine;
    }

    // Create original tasks for nodes
    create_tasks(nodes, a, b, ncomp);

    // Enter loop for managing all nodes
    double result = 0.0;
    do
    {
        // Receive all availible results
        fd_set fdset = {};
        FD_ZERO(&fdset);

        int fdSize = 0;
        for (long i = 0; i < ncomp; ++i)
        {
            if (deque_size(nodes[i].tasks) > 0 && nodes[i].state == STATE_OK)
            {
                fdSize++;
                FD_SET(nodes[i].socket, &fdset);
            }
        }

        if (fdSize > 0)
        {
            int ret = select(FD_SETSIZE, &fdset, NULL, NULL, NULL);
            if (ret < 0)
            {
                perror("Error while polling clients: ");
                goto exit_server_routine;
            }

            if (ret != 0) // somebody wants to send something
            {
                for (long i = 0; i < ncomp; ++i)
                {
                    if (FD_ISSET(nodes[i].socket, &fdset) && deque_size(nodes[i].tasks) > 0) // all nodes on set are already STATE_OK
                    {
                        // receive results one by one, consider node is ready == only one result is ready
                        printf("Receiving task result from node %ld\n", i);

                        double tmpResult = 0.0;
                        ssize_t msgBytes = recv(nodes[i].socket, &tmpResult, sizeof(tmpResult), 0);
                        
                        if (msgBytes != sizeof(tmpResult))
                        {
                            printf("Node %ld failed to send results\n", i);
                            printf("Required size: %ld, actual size %ld\n", sizeof(tmpResult), msgBytes);

                            nodes[i].state = STATE_TIMEOUT;
                            printf("Node %ld has been marked as timeout\n", i);
                            
                            break;
                        }
                        else
                        {
                            printf("Computer %ld; Size: %lu; Data: %g\n", i, msgBytes, tmpResult);
                            result += tmpResult;

                            if (deque_pop_front(nodes[i].tasks, NULL) < 0)
                            {
                                printf("Failed to receive tasks: deque error!\n");
                                ret_code = -1;
                                goto exit_server_routine;
                            }
                        }
                    }
                }
            }
        }

        // If there are nodes that are timeout, redistribute their tasks and finally mark them dead
        for (long i = 0; i < ncomp; ++i)
        {
            if (nodes[i].state == STATE_TIMEOUT)
            {
                int tasksToRedistribute = deque_size(nodes[i].tasks);
                printf("Redistributing node %ld tasks to everyone else\n", i);
                printf("Tasks to redistribute: %d\n", tasksToRedistribute);

                size_t countAlive = 0;
                for (long j = 0; j < ncomp; ++j)
                    if (nodes[j].state == STATE_OK)
                        countAlive++;
                
                if (countAlive == 0)
                {
                    printf("Failed to redistribute tasks, all nodes are dead!\n");
                    ret_code = -1;
                    goto exit_server_routine;
                }

                for (int j = 0; j < tasksToRedistribute; ++j)
                {
                    task_t task = {};
                    if (deque_pop_back(nodes[i].tasks, &task) < 0)
                    {
                        printf("Failed to redistribute tasks, tasks queue error!\n");
                        ret_code = -1;
                        goto exit_server_routine;
                    }
                    create_tasks(nodes, task.a, task.b, ncomp);
                }

                nodes[i].state = STATE_DEAD;
                printf("Node %ld has been marked as dead\n", i);
            }
        }
    } 
    while(count_all_tasks(nodes, ncomp) > 0);

    printf("Result: %g\n", result);

exit_server_routine:
    for (int i = 0; i < ncomp; ++i)
        destroy_node(&nodes[i]);

    free(nodes);
    return ret_code;
}

int main(int argc, char* argv[])
{
    if (argc != 4) 
    {
        printf("Usage: ./%s <ncomp> <start> <end>\n", argv[0]);
        return EXIT_FAILURE;
    }

    long ncomp = 0;
    if (input(argv[1], &ncomp) < 0 || ncomp <= 0)
    {
        printf("Error! Wrong input values\n");
        return -1;
    }

    long a = 0, b = 0;
    if (input(argv[2], &a) < 0 || input(argv[3], &b) < 0)
    {
        printf("Error! Wrong input values\n");
        return -1;
    }

    check_error_return(server_routine(ncomp, a, b));
    return 0;
}