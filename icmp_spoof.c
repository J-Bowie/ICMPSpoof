#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

unsigned short get_checksum(unsigned short *buf, int len)
{
  unsigned long sum = 0;

  while (len > 1) {
	sum += *buf++; // *((unsigned short*) x) to cast
        len -= 2;
  }

  if (len) // left over byte
	sum += (unsigned short) *(unsigned char*) buf;

  sum = (sum >> 16) + (sum &0xffff);
  sum += (sum >> 16);
  return (unsigned short) (~sum); //NOT operation to bitwise
}

int main(int argc, char **argv)
{
  int sd;
  struct sockaddr_in sin;
  char buf[1024];

  // Create the IP/ICMP headers and attach to the buffer
  struct ip *ip = (struct ip *)buf;
  struct icmp *icmp = (struct icmp *) (ip + 1);
  const int on = 1;

  // Allocate buffer size
  bzero(buf, sizeof(buf)); 
  
  // IP header
  ip->ip_v = 4;
  ip->ip_hl = 5;
  ip->ip_tos = 0;
  ip->ip_len = htons(sizeof(buf));
  ip->ip_id = 0;
  ip->ip_off = htons(0);
  ip->ip_ttl = 255; // 255 TTL
  ip->ip_p = 1; // ICMP
  ip->ip_sum = 0; // Don't care about this
  ip->ip_src.s_addr = inet_addr("172.16.141.187");
  ip->ip_dst.s_addr = inet_addr("8.8.8.8");
  
  // ICMP header
  icmp->icmp_type = ICMP_ECHO;
  icmp->icmp_code = 0;
  icmp->icmp_id = htons(50179);
  icmp->icmp_seq = htons(0x0);
  icmp->icmp_cksum = get_checksum((unsigned short *)icmp, 8);
  
  /* Create a raw socket with IP protocol. The IPPROTO_RAW parameter
   * tells the sytem that the IP header is already included;
   * this prevents the OS from adding another IP header.  */
  sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
  if (sd < 0) {
    perror("socket() error"); exit(-1);
  }
  
  /* This data structure is needed when sending the packets
   * using sockets. Normally, we need to fill out several
   * fields, but for raw sockets, we only need to fill out
   * this one field */
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = ip->ip_dst.s_addr;
  

  /* Send out the IP packet.
   * ip_len is the actual size of the packet. */
  if (sendto(sd, buf, sizeof(buf), 0, (struct sockaddr *)&sin, 
	     sizeof(sin)) < 0)  {
    perror("sendto() error"); exit(-1);
  }
  
  return 0;
}