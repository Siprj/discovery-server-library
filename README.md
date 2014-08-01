discovery-server-library
========================

Small library for implementing discovery protocol in to some programs controlling embedded embedded devices.

Protocol is rather simple. Server is sending broadcast messages and receiving response with addresses
(one device can response for more devices). All messages are sent through UDP.

Message flow
------------

1. Server send discover broadcast message: \<\<DISCOVERY\>\>
2. Device which receive discovery message send response in format: \<\<DISCOVERY: xxx.xxx.xxx.xxx\>\>
