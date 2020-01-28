{
  "location" : "scorn/cityguard",
  "rules": [
  {
  "match" : ["chain"],
  "pre" : [["quest", "scorn/GatePassword", "30"]],
  "post" : [["connection", "12"]],
  "msg" : ["Well, you know the password, so you must be ok.\nPass Friend.."]
  }, {
  "match" : ["chain"],
  "pre" : [],
  "post" : [["quest", "scorn/GatePassword", "30"], ["connection", "12"]],
  "msg" : ["Well, you know the password, so you must be ok.\nPass Friend..."]
  }, {
  "include" : ["gateguardcommon.msg"]
  }
]}
