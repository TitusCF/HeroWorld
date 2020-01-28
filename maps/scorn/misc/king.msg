{
  "location" : "scorn/nobility",
  "rules": [
  {
  "match" : ["*"],
  "pre" : [["quest", "scorn/Aristocracy", "90"]],
  "post" : [],
  "msg" : ["Greetings Prince $you"]
  },{
  "include" : ["/scorn/misc/king_archduke.msg"],
  "pre" : [["quest", "scorn/Aristocracy", "80"]]
  },{
  "include" : ["/scorn/misc/king_duke.msg"],
  "pre" : [["quest", "scorn/Aristocracy", "70"]]
  },{
  "include" : ["/scorn/misc/king_count.msg"],
  "pre" : [["quest", "scorn/Aristocracy", "60"]]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/Aristocracy", "10"]],
  "post" : [],
  "msg" : ["What are you doing here? I don't speak with lesser nobles, go speak to my magistrate he'll deal with you."]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["What's this? A commoner!\n Begone from my sight, vulgar little creature."]
  }
]}