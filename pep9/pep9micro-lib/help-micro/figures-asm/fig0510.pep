;File: fig0510.pep
;Computer Systems, Fifth edition
;Figure 5.10
;
         LDBA    'H',i       ;Output 'H'
         STBA    0xFC08,d
         LDBA    'i',i       ;Output 'i'
         STBA    0xFC08,d
         STOP
         .END
