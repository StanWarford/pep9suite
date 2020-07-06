;File: fig0514a.pep
;Computer Systems, Fifth edition
;Figure 5.14(a)
;
         LDBA    0x0013,d    
         STBA    0xFC08,d    
         LDBA    0x0014,d    
         STBA    0xFC08,d    
         LDBA    0x0006,d    
         STBA    0xFC08,d    
         STOP                
         .ASCII  "Pun"       
         .END                  
