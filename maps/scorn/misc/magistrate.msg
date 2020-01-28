{
  "location" : "scorn/nobility",
  "rules": [
  {
  "match" : ["farnass"],
  "pre" : [],
  "post" : [],
  "msg" : ["Ha, the cook? You can find him in the kitchen, near the castle.\n\nThe kitchen password is 'clickety'"]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/Aristocracy", "=90"]],
  "post" : [],
  "msg" : ["I am unworthy to be in your presence your highness, please, go upstairs and speak to the King directly."]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/Aristocracy", "=80"]],
  "post" : [],
  "msg" : ["I am unworthy to be in your presence your grace, please, go upstairs and speak to the King directly."]
  },{
  "include" : ["magistrate_duke.msg"],
  "pre" : [["quest", "scorn/Aristocracy", "=70"]]
  },{
  "include" : ["magistrate_count.msg"],
  "pre" : [["quest", "scorn/Aristocracy", "=60"]]
  },{
  "include" : ["magistrate_earl.msg"],
  "pre" : [["quest", "scorn/Aristocracy", "=50"]]
  },{
  "include" : ["magistrate_marquis.msg"],
  "pre" : [["quest", "scorn/Aristocracy", "=40"]]
  },{
  "include" : ["magistrate_baron.msg"],
  "pre" : [["quest", "scorn/Aristocracy", "=30"]]
  },{
  "include" : ["magistrate_baronet.msg"],
  "pre" : [["quest", "scorn/Aristocracy", "=20"]]
  },{
  "include" : ["magistrate_knight.msg"],
  "pre" : [["quest", "scorn/Aristocracy", "=10"]]
  },{
  "include" : ["magistrate_civilian.msg"]
  }
]}