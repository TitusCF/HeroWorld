{
  "location" : "scorn/igor",
  "rules": [
  {
  "match" : ["*"],
  "pre" : [["quest", "scorn/Kelthior", "40"]],
  "post" : [],
  "msg" : ["Yes $you, anything I can do to help you?"],
  "replies" : []
  },{
  "match" : ["yes"],
  "pre" : [["npctoken", "step", "2"], ["archininventory", "skill_inscription"]],
  "post" : [["quest", "scorn/Kelthior", "40"]],
  "msg" : ["Very well!\n\nYou may talk to my master."],
  "replies" : []
  },{
  "match" : ["yes"],
  "pre" : [["npctoken", "step", "2"]],
  "post" : [["setnpctoken", "step", "0"]],
  "msg" : ["Somehow I'm not sure you actually know... Please come back when you really know how to write."],
  "replies" : []
  },{
  "match" : ["no"],
  "pre" : [["npctoken", "step", "2"]],
  "post" : [["setnpctoken", "step", "0"]],
  "msg" : ["Then you need to learn before you can get on top of the tower..."],
  "replies" : []
  },{
  "match" : ["yes"],
  "pre" : [["npctoken", "step", "1"]],
  "post" : [["setnpctoken", "step", "2"]],
  "msg" : ["Then tell me, do you know how to write?"],
  "replies" : [["yes", "Yes.", "1"], ["no", "No, unfortunately.", "1"]]
  },{
  "match" : ["no"],
  "pre" : [["npctoken", "step", "1"]],
  "post" : [["setnpctoken", "step", "0"]],
  "msg" : ["Well, then please come back later."],
  "replies" : []
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [["setnpctoken", "step", "1"]],
  "msg" : ["Hello again $you, this is the last floor before you can see my master.\n\nAre you ready for the final challenge?"],
  "replies" : [["yes", "Yes, please, I really want to reach the top.", "1"], ["no", "No, I'm tired, and I need a good bath before I try again to go upstairs...", "1"]]
  }
]}
