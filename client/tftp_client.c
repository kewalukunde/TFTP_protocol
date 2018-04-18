/* =====================================================================================
 *
 *       Filename:  tftp_client.c
 *        Created:  thusday 30 March 2017 02:48:48  IST
 *         Author:  KEWAL UKUNDE , kewalukunde@gmail.com
 *
 * =====================================================================================
 */

#include <tftp.h>

int main(int argc , char **argv)
{

	if (argc != 2)
		error(1, 0, "usage : ./client <server IP ADDRESS>");

    int sock_fd, c_size, file_fd, len;
    ssize_t r_byte;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;
    packet s_packet, r_packet;
	char *ip_address = argv[1];
	char cmd[10], *ptr;
    char *buff = calloc(50, sizeof(char));

    /* create udp socket */
    if ((sock_fd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
		error(1, errno, "socket failed");
	perror("socket:");

    /* Bind it to a particular IP address & port */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip_address);
    serv_addr.sin_port = htons(SERVER_PORT);

    while (1)
    {
        memset(&s_packet, 0, sizeof(s_packet));
        memset(&r_packet, 0, sizeof(r_packet));
        printf("ENTER REQUST: ");
        scanf("%[^\n]",buff);
		getchar();
		strcpy(cmd, strtok(buff, " "));
		ptr = strtok(NULL, " ");
		if (ptr != NULL)
			strcpy(s_packet.filename, ptr);

        /* get operetion */
        if (!strcmp(cmd, "get")) {
            s_packet.b_num = 0;
            s_packet.e_code = 0;
            memset(s_packet.data,0,DATA_SIZE);
            memset(s_packet.msg,0,20);

            /* seting as read request */
            s_packet.opcode = READ_REQ;

            /* checking file already exist or not*/
            if ((file_fd = open(s_packet.filename, O_WRONLY | O_CREAT | O_EXCL,
							S_IRWXU)) == -1) {
                if (errno == EEXIST) {
                    printf("\nFile already exist...\n\n");
                    continue;
                }
            }

            /* sending request */
            if(( c_size = sendto(sock_fd, (void*)&s_packet, sizeof(s_packet), 0,
							(struct sockaddr*)&serv_addr, sizeof(serv_addr))) == -1)
				error(1, errno, "sendto failed");

            if(c_size)
                printf("\nrequsted for file '%s'..!\n\n", s_packet.filename);
            else
                printf("Error\n");

            /* reciving data with error bit */
            cli_len = sizeof(cli_addr);
            if ((recvfrom(sock_fd, (void*)&r_packet, sizeof(r_packet), 0,
							(struct sockaddr *)&cli_addr, &cli_len)) == -1)
				error(1, errno, "recvfrom failed");

            /* checking error code */
            if (r_packet.e_code != 0) {
                printf("%s\n\n", r_packet.msg);
				if (remove(s_packet.filename) == -1)
					printf("failed to delete file");
			} else {
                /* loop to process data */
                while (1) {
                    //printf("\nr_packet.data = %s\n",r_packet.data);
                    len = strlen(r_packet.data);
                    r_byte = write(file_fd, r_packet.data, strlen(r_packet.data));

                    printf("recived %2dth data packet \t", r_packet.b_num);

                    s_packet.b_num = r_packet.b_num;
                    s_packet.e_code = 0;
                    memset(s_packet.filename, 0, 15);
                    memset(s_packet.data, 0, DATA_SIZE);
                    memset(s_packet.msg, 0, 20);

                    /* sending ack */
                    if(( c_size = sendto(sock_fd, (void*)&s_packet, sizeof(s_packet), 0,
									(struct sockaddr*)&serv_addr, sizeof(serv_addr))) == -1)
						error(1, errno, "sendto failed");
					else
                        printf("Sending acknowledge packet %2d\n",s_packet.b_num);

                    /* checking data sending comlited or not */
                    if ((r_byte  < DATA_SIZE) || (len < DATA_SIZE)) {
                        printf("\n\ncomplited data transfer..!\n\n");
                        close(file_fd);
                        break;   
                    }

                    memset(buff, 0, DATA_SIZE);

                    /* reciving data */
                    cli_len = sizeof(cli_addr);
                    if ((recvfrom(sock_fd, (void*)&r_packet, sizeof(r_packet), 0,
									(struct sockaddr *)&cli_addr, &cli_len)) == -1)
						error(1, errno, "recvfrom failed");
                }
            }
        } else if(!strcmp(cmd, "put")) {
            s_packet.b_num = 0;
            s_packet.e_code = 0;
            memset(s_packet.data, 0, DATA_SIZE);
            memset(s_packet.msg, 0, 20);

            s_packet.opcode = WRITE_REQ;

            if ((file_fd = open(s_packet.filename, O_RDONLY)) == -1)
				printf("\nfile not available\n\n");
            else {
                /* sending data to server */
                if(( c_size = sendto(sock_fd, (void*)&s_packet, sizeof(s_packet), 0,
								(struct sockaddr*)&serv_addr, sizeof(serv_addr))) == -1)
					error(1, errno, "recvfrom failed");

                if(c_size)
                    printf("\nrequsted for file '%s'..!\n\n", s_packet.filename);
                else
                    printf("Error\n");
                
                /* reciving first ack from server */
                cli_len = sizeof(cli_addr);
                if ((recvfrom(sock_fd, (void*)&r_packet, sizeof(r_packet),0,
								(struct sockaddr *)&cli_addr, &cli_len)) == -1)
					error(1, errno, "recvfrom failed");

                /* error checking if it is*/
                if (r_packet.e_code != 0)
					printf("%s...\n\n", r_packet.msg);
                else {
                    s_packet.b_num = 0;

                    while (1) {
                        /* reading data */
                        r_byte = read(file_fd, (void*)buff, DATA_SIZE);

                        strcpy(s_packet.data,buff);
                        memset(s_packet.filename,0,15);
                        memset(s_packet.msg,0,20);
                        s_packet.e_code = 0;
                        s_packet.b_num++;

                        ///printf("\nr_packet->data : %s\n",s_packet.data);

                        /* sending data */
                        if((c_size = sendto(sock_fd, (void *)&s_packet, sizeof(s_packet), 0,
										(struct sockaddr*)&cli_addr, cli_len)) == -1)
							error(1, errno, "sendto failed");
                        else
                            printf("sending %2dth data packet \t", r_packet.b_num);

                        memset(buff, 0, DATA_SIZE);
                        
                        /* reciving acknolegedment */
                        cli_len = sizeof(cli_addr);
                        if ((recvfrom(sock_fd, (void*)&r_packet, sizeof(r_packet), 0,
										(struct sockaddr *)&cli_addr, &cli_len)) == -1)
							error(1, errno, "recvfrom failed");
                        else {
                            r_packet = (packet)r_packet;
                            printf("recived acknowledge packet %d\n",r_packet.b_num);
                        }

                        if (r_byte < DATA_SIZE) {
                            //printf("\nr_packet->data : %s\n",s_packet.data);
                            printf("\ncomplited data transfer..!\n\n");
                            close(file_fd);
                            break;   
                        }
                    }

                }
            }
        } else if(!strcmp(cmd, "bye"))
            exit(0);
        else if(!strcmp(cmd, "help")) {
            printf("\nUsage : get <filename>\n");
            printf("Usage : put <filename>\n");
		} else
            printf("\nInvalied command..!!\n");

    }
    close(sock_fd);
}
