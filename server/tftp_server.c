/* =====================================================================================
 *
 *       Filename:  tftp_server.c
 *        Created:  Thursday 30 March 2017 11:56:06  IST
 *         Author:  KEWAL UKUNDE , kewalukunde@gmail.com
 *
 * =====================================================================================
 */

#include <tftp.h>

int main(void)
{
	int len, sock_fd, c_size, file_fd;
    char *buff = calloc(DATA_SIZE, sizeof(char));
    ssize_t r_byte;
	int ret;
	struct hostent IP_info;

	//char *ip_address = get_ip_address();

    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;
    packet r_packet, s_packet;

    /* create tcp socket */
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd == -1)
		error(1, errno, "socket failed");
    perror("socket");

    /* Bind it to a particular IP address & port */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERVER_PORT);

	printf("MY_IP : %s\n", inet_ntoa(INADDR_ANY);
    ret = bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (ret == -1)
		error(1, errno, "bind failed");
    perror("bind");

    while (1) {
        printf("\nserver is waiting...\n");

        /* reciving requast from client */
        cli_len = sizeof(cli_addr);
        if ((recvfrom(sock_fd, (void*)&r_packet, sizeof(r_packet), 0,
						(struct sockaddr *)&cli_addr, &cli_len)) == -1)
			error(1, errno, "recvfrom failed");

        r_packet = (packet)r_packet;
        printf("opcode : %d\n", r_packet.opcode);
        printf("file name : %s\n\n", r_packet.filename);

		switch (r_packet.opcode) {
		/* read request from client */
		case READ_REQ : 
			if ((file_fd = open(r_packet.filename, O_RDONLY)) == -1) {
				printf("\nError in opperation!! sending Error\n");
				memset(s_packet.filename, 0, 15);
				memset(s_packet.data, 0, DATA_SIZE);
				strcpy(s_packet.msg, "File not present!");
				s_packet.b_num = 0;
				s_packet.e_code = FILE_NOT_FOUND;

				/* sending error for request */
				if((c_size = sendto(sock_fd, (void *)&s_packet, sizeof(s_packet), 0,
								(struct sockaddr*)&cli_addr, cli_len)) == -1)
					error(1, errno, "sendto failed");
			} else {
				s_packet.b_num = 0;
				/* loop to send data */
				while (1) {
					r_byte = read(file_fd,(void*)buff,DATA_SIZE);

					strcpy(s_packet.data,buff);
					memset(s_packet.filename, 0, 15);
					memset(s_packet.msg, 0, 20);
					s_packet.e_code = 0;
					s_packet.b_num++;

					//printf("\nr_packet->data : %s\n",s_packet.data);

					/* sending data packet */
					if((c_size = sendto(sock_fd, (void *)&s_packet, sizeof(s_packet),
									0, (struct sockaddr*)&cli_addr, cli_len)) == -1)
						error(1, errno, "sendto failed");
					else
						printf("sending %2d data packet\t", s_packet.b_num);

					memset(buff, 0, DATA_SIZE);

					/* reciving the ack bit */
					cli_len = sizeof(cli_addr);
					if ((recvfrom(sock_fd, (void*)&r_packet, sizeof(r_packet), 0,
									(struct sockaddr *)&cli_addr, &cli_len)) == -1)
						error(1, errno, "recvfrom failed");
					else {
						r_packet = (packet)r_packet;
						printf("\trecived %2dth acknowledge packet\n", r_packet.b_num);
					}

					/* checking data transfer is complited or not */
					if (r_byte < DATA_SIZE) {
						printf("\n\ncomplited data transfer..!\n");
						close(file_fd);
						break;   
					}
				}
			}
			break;

			/* write request to server */
		case WRITE_REQ : 
			if ((file_fd = open(r_packet.filename, O_WRONLY | O_CREAT | O_EXCL,
							S_IRWXU)) == -1) {
				/* error maessage sending */
				printf("\nError in opperation!! sending Error\n");
				if (errno == EEXIST) {
					memset(s_packet.filename,0,15);
					memset(s_packet.data, 0, DATA_SIZE);
					strcpy(s_packet.msg, "File already exist");
					s_packet.b_num = 0;
					s_packet.e_code = FILE_EXIST;

					if((c_size = sendto(sock_fd, (void *)&s_packet, sizeof(s_packet),
									0, (struct sockaddr*)&cli_addr, cli_len)) == -1)
						error(1, errno, "sendto failed");
				}
			} else {
				s_packet.b_num = 0;
				s_packet.e_code = 0;
				memset(s_packet.filename, 0, 15);
				memset(s_packet.data, 0, DATA_SIZE);
				memset(s_packet.msg, 0, 20);

				/* first ack sending with no error */
				if(( c_size = sendto(sock_fd, (void*)&s_packet, sizeof(s_packet), 0,
								(struct sockaddr*)&cli_addr, cli_len)) == -1)
					perror("sendto:");

				while (1) {
					/* reciving data from client */
					cli_len = sizeof(cli_addr);
					if ((recvfrom(sock_fd, (void*)&r_packet, sizeof(r_packet), 0,
									(struct sockaddr *)&cli_addr, &cli_len)) == -1)
						error(1, errno, "recvfrom failed");
					else
						printf("recived data packet block %2d \t", r_packet.b_num);

					len = strlen(r_packet.data);
					//printf("\nr_packet.data = %s\n",r_packet.data);
					/* writing data to file */
					r_byte = write(file_fd,r_packet.data,strlen(r_packet.data));
					s_packet.b_num = r_packet.b_num;
					s_packet.e_code = 0;
					memset(s_packet.filename, 0, 15);
					memset(s_packet.data, 0, DATA_SIZE);
					memset(s_packet.msg, 0, 20);

					/* sending back ack */
					if((c_size = sendto(sock_fd, (void*)&s_packet, sizeof(s_packet),
									0,(struct sockaddr*)&cli_addr, cli_len)) == -1)
						perror("sendto:");
					else
						printf("sending acknowledge packet %2d\n",s_packet.b_num);

					/* recived data less than 450 then break */
					if ((r_byte < DATA_SIZE) || (len < DATA_SIZE)) {
						printf("\n\nData transfer complited..!\n");
						close(file_fd);
						break;
					}
				}
			}
			break;
		default :
			break;
		}

	}
	/* close the socket now */
	close(sock_fd);
}
