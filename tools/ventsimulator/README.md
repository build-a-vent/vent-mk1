ventsimulator : A simulator of a vent for GUI development

synopsis : ventsimulator <portnumber> <ventname> <vent-mac-xor-value>
  default port 1111
  default name vent-1
  default mac 35:e3:ce, is xored with the vent-mac-xor-value

ventsimulator will connect to the given port and wait for uni/broadcasts with json `{ "cmd":"scan", "seq":<nnn> }` contents

upon reception of a scan order ventsimul will reply with a json string containing `{ "req":"scan", cmd":"stat", "seq":<nnn> ,"var":value ... }`

The following numeric variables are supported

  | Name    | Contents | Lower limit | Upper limit |
  | :------ | -------- | ----------- | -----------
  | c_pip   | configured max inspiration press alarm limit \[mbar\] | 30 | 70 |
  | c_lip   | configured lowest inspiration pressure alarm limit \[mbar\] | -10 | 40 |
  | c_pep   | configured peak exspiration pressure alarm limit \[mbar\] | 5 | 40 |
  | c_lep   | configured lowest exspiration pressure alarm limit \[mbar\] | -10 | 30 |
  | c_flair | configured flow rate air ml/sec *to be configured by config app only* | 100 | 1500 |
  | c_flo2  | configured flow rate o2 ml/sec *to be configured by config app only*  | 100 |  1500 |
  | c_airt  | configured air time ms (approx 240ml) THIS makes the air part of tidal volume | 50 | 1500 |
  | c_o2t   | configured o2 time ms (approx 240ml)  THIS makes the oxygen part of tidal volume | 50 | 1500 |
  | c_inspt | configured inspiration. time  in ms before releasing outflow valve block to PEEP | 500 | 3000 |
  | c_cyclt | configured cyle time (20 cycles / min) | 2000 | 4000 |
  | c_wtemp | configured heater water temp \[degC\] | 30 | 40 |
  | a_eep   | actual end exspir. press last cycle  *NOT EDITABLE*
  | a_wtemp | actual heater water temp *NOT EDITABLE*
  | fio2_pct| O2-Contents, for compatibility with APP-Mk1
  | b_rat   | Inspir/Cycletime Ratio, for compatibility with APP-Mk1
  | cyc_min | Breathing cycles per minute for compatibility with APP-Mk1
  | tidal_ml| Tidal volume, for compatibility with APP-Mk1

The ventilator can modify those settings with a json string `{ "cmd":"save", "seq":<nnnn>, "<varname>":<value> ...}` and will get as a reply 
`{ "req":"save", cmd":"AKK", "seq":<nnnn>, "<varname>":<value> ...}`




CREDITS : Krzysztof Gabis https://github.com/kgabis/parson

HOWTO make : unpack into a directory, type 'make'. You will need gcc for your machine installed
