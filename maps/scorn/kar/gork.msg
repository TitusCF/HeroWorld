{
  "location" : "gork_quest",
  "rules": [
  {
  "match" : ["hi","hello"],
  "pre" : [],
  "post" : [["settoken", "gork_speak","hoard"],["quest", "scorn/GorksTreasure","20"]],
  "msg" : ["Youse want Gork's hoards?  I crush you..."],
  "replies": [["gork", "You are Gork, I presume?"], ["hoards", "Where are these hoards?"]]
  }, {
  "match" : ["gork","hoard","yes","want"],
  "pre" : [["token", "gork_speak","hoard","key","pulp"]],
  "post" : [["settoken", "gork_speak","mork"]],
  "msg" : ["Gork not like you...  I not as nice as Mork."],
  "replies" : [["mork", "Who is Mork?"]]
  }, {
  "match" : ["mork","nice"],
  "pre" : [["token", "gork_speak","friend","key","mork","tower","treasure"]],
  "post" : [["settoken", "gork_speak","friend"]],
  "msg" : ["Mork be Gork's friend.  Mork live in tower."],
  "replies" : [["friend", "Are you good friends?"],["tower", "Which tower would that be?"]]
  }, {
  "match" : ["friend"],
  "pre" : [["token", "gork_speak","friend","tower"]],
  "post" : [["settoken", "gork_speak","treasure"]],
  "msg" : ["Mork share Gork's treasure."],
  "replies" : [["share", "Can I share your treasure too?"]]
  }, {
  "match" : ["tower"],
  "pre" : [["token", "gork_speak","friend","tower"]],
  "post" : [["settoken", "gork_speak","tower"]],
  "msg" : ["Mork live in tower.  Mork big, Mork friend."],
  "replies" : [["friend", "Are you good friends?"]]
  }, {
  "match" : ["share","treasure"],
  "pre" : [["quest", "scorn/GorksTreasure","40"]],
  "post" : [["settoken", "gork_speak","key"]],
  "msg" : ["Mork gave you key to open treasure door? Oh well, Gork assume Gork's friend knows what he is doing."]
  }, {
  "match" : ["share","treasure"],
  "pre" : [["token", "gork_speak","treasure"]],
  "post" : [["settoken", "gork_speak","key"],["quest", "scorn/GorksTreasure","30"]],
  "msg" : ["Only Gork and Mork can open treasure door."],
  "replies" : [["open", "Will you open the treasure door for me Gork?"]]
  }, {
  "match" : ["crush","door","open","treasure"],
  "pre" : [["token", "gork_speak","hoard","key"]],
  "post" : [["settoken", "gork_speak","pulp"]],
  "msg" : ["Gurrr... Gork beat you to pulp!"]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post": [["settoken", "gork_speak","*"]],
  "msg" : ["Gork kill you!"]
  }
]}
