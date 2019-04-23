// test-badrouterduplicate-ping.clcik

// You should see, printed to standard error, a sequence of "icmp echo"
// printouts intermixed with "ping :: ICMPPingSource" receive reports.

define($DEV ens33, $DADDR 8.8.8.8, $GW $DEV:gw, $METHOD PCAP,
       $LIMIT -1, $INTERVAL 1)

FromDevice($DEV, SNIFFER false)
	-> c :: Classifier(12/0800, 12/0806 20/0002)
	-> CheckIPHeader(14)
	-> ip :: IPClassifier(icmp echo-reply)
  -> q1 :: Queue()
	-> BadrouterDrop(PROB 0.5)
	-> ping :: ICMPPingSource($DEV, $DADDR, INTERVAL $INTERVAL,
				  LIMIT $LIMIT, STOP true)
	-> SetIPAddress($GW)
	-> arpq :: ARPQuerier($DEV)
	-> IPPrint
	-> q :: Queue()
-> ToDevice($DEV);

arpq[1]	-> q;

c[1]	-> [1] arpq;
