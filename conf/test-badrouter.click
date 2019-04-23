// test-badrouter.click

FastUDPSource(100000, 500000, 60, 0:0:0:0:0:0,
              1.0.0.1, 1234, 1:1:1:1:1:1, 2.0.0.2, 1234)
    -> BadrouterCounter(LOG_ID 0)
    -> DelayUnqueue(0)
    -> Queue()
    -> BandwidthShaper(131072B/s)
    -> BadrouterCounter(LOG_ID 1)
-> Discard;
