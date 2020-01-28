{
  "rules": [
  {
  "match" : ["demon", "euthville", "portal", "broken", "threat", "yes"],
  "pre" : [["quest", "scorn/DemonPit", "10"], ["item", "broken gateway"], ["token", "question", "demon"]],
  "post" : [["takeitem", "broken gateway"], ["quest", "scorn/DemonPit", "50"], ["givecontents", "Count Reward"], ["settoken", "question", "0"]],
  "msg" : ["Congratulations on completing your quest.  Kneel and receive your accolade.....\n You are now a Count!\n Also, please take this fine sword as a reward. May it serve you well!"]
  },{
  "match" : ["demon", "euthville", "portal", "broken", "threat", "yes"],
  "pre" : [["quest", "scorn/DemonPit", "10"], ["token", "question", "demon"]],
  "post" : [["settoken", "question", "0"]],
  "msg" : ["Where is it then?"]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/DemonPit", "10"]],
  "post" : [["settoken", "question", "demon"]],
  "msg" : ["Have you returned with proof that the demons in Euthville are no longer a threat?"],
  "replies": [["no", "I am still investigating the demon menace."], ["yes", "Yes, I return with proof of their destruction."]]
  },{
  "match" : ["no"],
  "pre" : [],
  "post" : [],
  "msg" : ["Very well then Lord $you, do return here if you change your mind."]
  },{
  "match" : ["yes", "service", "Scorn"],
  "pre" : [],
  "post" : [["quest", "scorn/DemonPit", "10"]],
  "msg" : ["His Majesty, the King of Scorn, is concerned with the recent influx of demons in the ruins of the town north of Scorn.\nReports have it that the demons are issuing from a filthy pit in the ground.\n Go and investigate, and eliminate the threat if possible.\n Return with a token of your victory."]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hail Lord $you, would you be prepared to be of service to Scorn once again?"],
  "replies": [["no", "Not right now"], ["yes", "Of course, I would be delighted to serve this great city"]]
  }
]}