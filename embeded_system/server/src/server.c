#include "server.h"
#include "util.h"

void sv_listen(const sv_conf *args, int write_pipe) {
  long long ts = util_ts();
  int sockfd;
  struct sockaddr_in servaddr, cliaddr;

  // Creating socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // Filling server information
  servaddr.sin_family = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(args->port);

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  int len, n;

  len = sizeof(cliaddr); // len is value/result
  while (1) {
    printf("%i\tchild is listening\n", args->pid);
    char buffer[args->max_line];
    n = recvfrom(sockfd, (char *)buffer, args->max_line, MSG_WAITALL,
                 (struct sockaddr *)&cliaddr, &len);
    if (n > 0) {
      printf("%i\treceived from {%d:%d}: %s\n", args->pid,
             cliaddr.sin_addr.s_addr, cliaddr.sin_port, buffer);
      // Set buffer as `null terminated string`
      sv_motion data;
      memcpy(&data, buffer, sizeof(sv_motion));

      if (util_mkbit(data.direction, data.timestamp) == data.c_bit) {
        printf("%i\tvalid data:{%d %lu}\n", args->pid, data.direction,
               data.timestamp);
      }

      // Writing to the pipe
      printf("%i\ttransferring to parent\n", args->pid);
      write(write_pipe, &data, sizeof(sv_motion));
    }
  }
}
