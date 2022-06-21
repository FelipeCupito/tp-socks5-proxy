#!/bin/bash
for i in {1..1000}; 
   do curl --limit-rate 1 --keepalive-time 60 --keepalive -x socks5h://127.0.0.1:1080 www.youtube.com & done
   echo $i