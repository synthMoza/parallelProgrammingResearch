#include <math.h>

#include <network.h>

// Function to integrate
// @param x the argument of the function
double function(double x) 
{
    double f = pow(x, 2);
    return f;
}

static int receive_ip_from_broadcast(struct sockaddr_in* client)
{
    // Wait for the message from the router (UDP)
    int sk_br = socket(AF_INET, SOCK_DGRAM, 0);
    check_error_return(sk_br);

    // Set socket's options
    int enable = 1;
    check_error_return(setsockopt(sk_br, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)));
    check_error_return(setsockopt(sk_br, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)));

    // Set up the struct for receiving any message
    bzero(client, sizeof(*client));
    client->sin_family = AF_INET;
    client->sin_port = htons(PORT);
    client->sin_addr.s_addr = htonl(INADDR_ANY);

    check_error_return(bind(sk_br, (struct sockaddr*) client, sizeof(*client)));

    // Wait for the server's message
    printf("Waiting for the server...\n");
    
    char buf[MAX_MSG_SIZE] = {};
    socklen_t socklen = sizeof(client);
    check_error_return(recvfrom(sk_br, buf, sizeof(client->sin_addr.s_addr), 0, (struct sockaddr*) client, &socklen));

    printf("Broadcast message accepted! Data: %s; Server IP: %s\n", buf, inet_ntoa(client->sin_addr));
    close(sk_br);
    
    return 0;
}

static int get_tcp_socket()
{
    // Initialize TCP socket
    int sk_tcp = socket(AF_INET, SOCK_STREAM, 0);
    check_error_return(sk_tcp);

    int enable = 1;
    check_error_return(setsockopt(sk_tcp, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)));
    
    return sk_tcp;
}

static int client_routine(long nthreads)
{
    // Receive broadcast message from server, extract IP address from it
    struct sockaddr_in client = {};
    check_error_return(receive_ip_from_broadcast(&client));

    // Now we have server's address, adjust struct settings to connect to it
    client.sin_family = AF_INET;
    client.sin_port = htons(PORT);

    int sk_tcp = get_tcp_socket();
    check_error_return(sk_tcp);

    // Connect to the server
    check_error_return(connect(sk_tcp, (struct sockaddr*) &client, sizeof(client)));
    printf("Connection to the server has been established!\n");

    // Send avalible amount of sockets
    check_error_return(send(sk_tcp, &nthreads, sizeof(nthreads), 0));

    // Run an infinite loop. While we can receive results from the server, calculate and send results
    task_t task;
    while (recv(sk_tcp, &task, sizeof(task), 0) > 0)
    {
        printf("Received new task. a = %g, b = %g\n", task.a, task.b);

        // Calculate the result
        double result = 0;
        check_error_return(thread_integrate(function, task.a, task.b, nthreads, &result));
        printf("Calculation finished. Result = %g\n", result);

        // Send results to the main computer
        ssize_t msgSize = send(sk_tcp, &result, sizeof(result), 0);
        check_error_return(msgSize);
        
        printf("Results has been sent. Message size: %ld\n", msgSize);
    }

    printf("Shutting down client...\n");
    
    close(sk_tcp);
    return 0;
}

int main(int argc, char* argv[]) 
{
    long nthreads = 0;

    if (argc != 2) 
    {
        printf("Usage: ./%s <nthreads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (input(argv[1], &nthreads) < 0 || nthreads <= 0)
    {
        printf("Error! Wrong input values\n");
        return -1;
    }

    check_error_return(client_routine(nthreads));
    return 0;
}