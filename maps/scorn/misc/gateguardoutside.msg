{
  "location" : "scorn/cityguard",
  "rules": [
  {
  "match" : ["chain"],
  "pre" : [["quest", "scorn/GatePassword", "30"]],
  "post" : [["connection", "22"]],
  "msg" : ["Well, you know the password, so you must be ok.\nPass Friend.."]
  },{
  "match" : ["chain"],
  "pre" : [],
  "post" : [["quest", "scorn/GatePassword", "30"], ["connection", "22"]],
  "msg" : ["Well, you know the password, so you must be ok.\nPass Friend..."]
  }, {
  "include" : ["gateguardcommon.msg"]
  }
]}



