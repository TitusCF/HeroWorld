{
  "location" : "gork_quest",
  "rules": [
  {
  "match" : ["gork"],
  "pre" : [["quest", "scorn/GorksTreasure", "30"]],
  "post" : [["settoken", "mork_speak","gork"]],
  "msg" : ["You know Gork?  You friend of Gork?"],
  "replies" : [["yes", "oh yes, Gork and I are great friends"], ["no", "Nope, Gork is no friend of mine"]]
  }, {
  "match" : ["yes","friend"],
  "pre" : [["quest", "scorn/GorksTreasure", "30"],["token", "mork_speak","gork"]],
  "post" : [["settoken", "mork_speak","key"]],
  "msg" : ["I bet Gork send you to get key.  Did he?"],
  "replies" : [["yes", "why, er yes, Gork wanted me to get the key and clean ou.. up, his treasure room"], ["no", "no, Gork didn't say anything about getting a key"]]
  }, {
  "match" : ["key","send","yes"],
  "pre" : [["quest", "scorn/GorksTreasure", "40"]],
  "post" : [],
  "msg" : ["I gave you key. Now you go."]
  },{
  "match" : ["key","send","yes"],
  "pre" : [["quest", "scorn/GorksTreasure", "30"],["token", "mork_speak","key"]],
  "post" : [["settoken", "mork_speak","0"],["quest", "scorn/GorksTreasure", "40"], ["giveitem", "gorks key"], ["connection", "9"]],
  "msg" : ["Ok, I give you key.  Here you go."]
  }, {
  "match" : ["hello","hi"],
  "pre" : [],
  "post" : [],
  "msg" : ["Geesh, kill all my friends, that's not very nice.\nWhere did you learn your manners?"],
  "replies" : [["friends", "You were friends with those creatures?"]]
  }, {
  "match" : ["friends"],
  "pre" : [],
  "post" : [],
  "msg" : ["They were just my pals."],
  "replies" : [["pals", "I'm sorry I killed your pals"]]
  }, {
  "match" : ["pals"],
  "pre" : [],
  "post" : [],
  "msg" : ["I'm tired of talking to you, you racist...\nGO AWAY!"]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Assassin!"]
  }
]}
