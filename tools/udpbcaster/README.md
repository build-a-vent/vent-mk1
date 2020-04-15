udpbcaster : A simple tool to simulate build-a-vent udp behaviour

synopsis : udpbcaster <Portnumber> <broadcaststring>

udpbcaster will broadcast the broadcaststring every 10 seconds from the (own_ip:port) to (broadcast:port)
while listening on (own_ip:port) for incoming udp datagrams. If a datagram from a foreign address arrives
udpbcaster will prepend the text in the telegram with "ACK:" and send it back to the address/port it came from

Can be tested with "UDP Terminal" on Android. Maybe not all ports are open to user programs in Android,
however I got it working on port1111

here in the repository is a zipped binary for 64-bit linux (works on ubuntu 18.04)