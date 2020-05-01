ventsimulator : A simulator of a vent for GUI development

synopsis : ventsimulator <Portnumber> <ventname>

ventsimulator will connect to the given port and wait for uni/broadcasts with json `{ "cmd":"scan", "seq":<nnn> }` contents

upon reception of a scan order ventsimul will reply with a json string containing `{ "req":"scan", cmd":"stat", "seq":<nnn> ,"var":value ... }`

The following numeric variables are supported

  | Name    | Contents |
  | :------ | -------- |
  | c_pip   | configured max inspiration press alarm limit |
  | c_lip   | configured lowest inspiration pressure alarm limit |
  | c_pep   | configured peak exspiration pressure alarm limit
  | c_lep   | configured lowest exspiration pressure alarm limit
  | c_flair | configured flow rate air ml/sec
  | c_flo2  | configured flow rate o2 ml/sec
  | c_airt  | configured air time ms (approx 240ml) THIS makes the air part of tidal volume
  | c_o2t   | configured o2 time ms (approx 240ml)  THIS makes the oxygen part of tidal volume
  | c_inspt | configured inspiration. time  in ms before releasing outflow valve block to PEEP
  | c_cyclt | configured cyle time (20 cycles / min)
  | c_wtemp | configured herater water temp
  | a_eep   | actual end exspir. press last cycle  NOT EDITABLE
  | a_wtemp | actual heater water temp NOT EDITABLE

The ventilator can modify those settings with a json string `{ "cmd":"set", "seq":<nnnn>, "<varname>":<value> ...}` and will get as a reply 
`{ "req":"set", cmd":"ack", "seq":<nnnn>, "<varname>":<value> ...}`



CREDITS : Krzysztof Gabis https://github.com/kgabis/parson

HOWTO make : unpack into a directory, type 'make'. You will need gcc for your machine installed
