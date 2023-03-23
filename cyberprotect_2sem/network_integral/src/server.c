#include <errno.h>
#include <network.h>

// The length to integrate on
const int integrate_a = 1;
const int integrate_b = 10e7;

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
    
    int maxpkt = 10;
    check_error_return(setsockopt(sk_tcp, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(maxpkt)));

    return 0;
}

void destroy_node(node_t* node)
{
    if (!node)
        return;
    
    if (node->socket != 0)
        close(node->socket);
    
    vector_destroy(&node->tasks);
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
    int nthrecv = 0; // receive threads

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
            printf("Accepted new computer! Number: %ld\n", naccepted++);
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
                printf("Number of threads: %d\n", nodes[i].threads);
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

static void print_task(const task_t* task)
{
    printf("task.a = %g\n", task->a);
    printf("task.b = %g\n", task->b);
}

// count all nodes tasks and return this value
static int count_all_tasks(const node_t* nodes, long ncomp)
{
    int count = 0;
    for (long i = 0; i < ncomp; ++i)
        if (nodes[i].state == STATE_OK)
            count += nodes[i].nreceive_tasks;
    

    return count;
}

static int create_tasks(node_t* nodes, double a, double b, int ncomp)
{
    // count threads of alive nodes
    int sum = 0;
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
        vector_push_back(&nodes[0].tasks, prev_task);
        print_task(&prev_task);
    }
    
    for (int i = 1; i < ncomp; ++i)
    {
        if (nodes[i].state == STATE_OK)
        {
            task_t task = {
                .a = prev_task.b,
                .b = a + nodes[i].threads * diff
            };

            print_task(&task);
            prev_task.b = task.b;
        }
    }
    
    return 0;
}

static int receive_results(int* sk_cl, long ncomp, double* result)
{
    int ret_code = 0;

    // Accept all results
    int* received = (int*) calloc(ncomp, sizeof(*received));
    check_zero_return(received);

    int nreceived = 0;
    while(1)
    {
        if (nreceived == ncomp)
            break;
        
        fd_set fdset = {};
        FD_ZERO(&fdset);

        struct timeval timeval = {
            .tv_usec = 0,
            .tv_sec = 10
        };

        for (int i = 0; i < ncomp; ++i)
        {
            // Add all computers to the file set
            if (received[i] == 0)
                FD_SET(sk_cl[i], &fdset);
        }

        int ret = select(FD_SETSIZE, &fdset, NULL, NULL, &timeval);
        if (ret < 0)
        {
            ret_code = -1;
            goto exit_receive_results;
        }

        if (ret == 0)
            break;

        for (int i = 0; i < ncomp; ++i) 
        {
            if (FD_ISSET(sk_cl[i], &fdset)) 
            {
                double tmp = 0;

                // Receive data from this client
                printf("Computer %d is ready to send results\n", i);
                ssize_t msgBytes = recv(sk_cl[i], &tmp, sizeof(tmp), 0);
                if (msgBytes == 0)
                {
                    // Node died, we need to do several things:
                    // 1) Don't expect it to reanimate from the dead, close connection imediately and remove from the waiting list
                    // 2) This task still needs to be done - give it to another node? or distribute between alive nodes
                }
                else if (msgBytes != sizeof(tmp))
                {
                    printf("Received result size is wrong! Must be %ld, actual size - %ld\n", sizeof(tmp), msgBytes);
                    ret_code = -1;
                    goto exit_receive_results;
                }

                printf("Computer %d; Size: %u; Data: %g\n", i, ret, tmp);
                received[i] = 1;
                
                nreceived++;
                *result += tmp;
            } 
        }
    }

    if (ncomp != nreceived) 
        ret_code = -1;

exit_receive_results:
    free(received);
    return ret_code;
}

static int server_routine(long ncomp)
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
        vector_init(&nodes[i].tasks);
        vector_reserve(&nodes[i].tasks, 1u);
        nodes[i].nreceive_tasks = 0;
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
    create_tasks(nodes, integrate_a, integrate_b, ncomp);

    // Enter loop for managing all nodes
    double result = 0.0;
    do
    {
        // Send all available tasks
        for (long i = 0; i < ncomp; ++i)
        {
            if (nodes[i].state == STATE_OK)
            {
                int isSendFailed = 0;
                printf("Sending %ld tasks to node %ld\n", vector_size(&nodes[i].tasks), i);
                
                for (size_t j = 0; j < vector_size(&nodes[i].tasks); ++j)
                {
                    task_t task = vector_elem(&nodes[i].tasks, j);
                    if (send(nodes[i].socket, &task, sizeof(task), 0) != sizeof(task))
                    {
                        printf("Failed to send task to node %ld\n", i);

                        nodes[i].state = STATE_TIMEOUT;
                        printf("Node %ld has been marked as timeout\n", i);
                        
                        isSendFailed = 1;
                        break;
                    }

                    nodes[i].nreceive_tasks++;
                }
                
                if (isSendFailed == 0)
                {
                    // all tasks has been sent, clear queue
                    vector_clear(&nodes[i].tasks);
                }
            }
        }

        // Receive all availible results
        for (long i = 0; i < ncomp; ++i)
        {
            if (nodes[i].state == STATE_OK)
            {
                printf("Receiving %d results from node %ld\n", nodes[i].nreceive_tasks, i);
                while (nodes[i].nreceive_tasks > 0)
                {
                    double tmpResult = 0.0;
                    if (recv(nodes[i].socket, &tmpResult, sizeof(tmpResult), 0) != sizeof(tmpResult))
                    {
                        printf("Node %ld failed to send results\n", i);

                        nodes[i].state = STATE_TIMEOUT;
                        printf("Node %ld has been marked as timeout\n", i);
                        
                        break;
                    }

                    nodes[i].nreceive_tasks--;
                }
            }
        }

        // If there are nodes that are timeout, redistribute their tasks and finally mark them dead
        for (long i = 0; i < ncomp; ++i)
        {
            if (nodes[i].state == STATE_TIMEOUT)
            {
                printf("Redistributing node %ld tasks to everyone else\n", i);
                for (size_t j = 0; j < vector_size(&nodes[i].tasks); ++j)
                {
                    task_t task = vector_elem(&nodes[i].tasks, j);
                    create_tasks(nodes, task.a, task.b, ncomp);
                }

                vector_clear(&nodes[i].tasks); // all tasks has been redistributed
                nodes[i].state = STATE_DEAD;
                printf("Node %ld has been marked as dead\n", i);
            }
        }
    } 
    while(count_all_tasks(nodes, ncomp) > 0);

    // Send data to all computers
    // for (int i = 0; i < ncomp; ++i)
    // {
    //     if (send(sk_cl[i], &task[i], sizeof(task[i]), 0) != sizeof(task[i]))
    //     {
    //         ret_code = -ENOMEM;
    //         goto exit_server_routine;
    //     }
    // }

    // double result = 0;
    // receive_results(sk_cl, ncomp, &result);
    printf("Result: %g\n", result);

exit_server_routine:
    for (int i = 0; i < ncomp; ++i)
        destroy_node(&nodes[i]);

    free(nodes);
    return ret_code;
}

int main(int argc, char* argv[])
{
    if (argc != 2) 
    {
        printf("Usage: ./%s <ncomp>\n", argv[0]);
        return EXIT_FAILURE;
    }

    long ncomp = 0;
    if (input(argv[1], &ncomp) < 0 || ncomp <= 0)
    {
        printf("Error! Wrong input values\n");
        return -1;
    }

    check_error_return(server_routine(ncomp));
    return 0;
}