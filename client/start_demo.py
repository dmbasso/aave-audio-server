from client import AudioServerInterface, PY3
import time

srv = AudioServerInterface()

with srv.packet() as pkt:
    pkt.output_set_frame(0,0)
    
time.sleep(5)

with srv.packet() as pkt:    
    pkt.output_set_frame(0,2**30)
    
time.sleep(5)

with srv.packet() as pkt:    
    pkt.output_set_frame(0,0)
    
time.sleep(5)

with srv.packet() as pkt:    
    pkt.output_set_frame(0,2**30)
    
time.sleep(5)

with srv.packet() as pkt:    
    pkt.output_set_frame(0,0)
    
time.sleep(5)

with srv.packet() as pkt:    
    pkt.output_set_frame(0,2**30)
    
time.sleep(5)

with srv.packet() as pkt:    
    pkt.output_set_frame(0,0)
