{
  "rules": [
  {
  "match" : ["wing", "wyvern", "snack", "slithselest", "yes"],
  "pre" : [["quest", "scorn/WyvernWing", "10"], ["item", "Slithselest's wing"], ["token", "question", "wing"]],
  "post" : [["takeitem", "Slithselest's wing"], ["quest", "scorn/WyvernWing", "50"], ["givecontents", "Earl Reward"], ["settoken", "question", "0"]],
  "msg" : ["Congratulations on completing your quest.\nKneel and receive your accolade.....\nYou are now an Earl!\n Also, please take this fine shield as a reward. May it serve you well!"]
  },{
  "match" : ["wing", "wyvern", "snack", "slithselest", "yes"],
  "pre" : [["quest", "scorn/WyvernWing", "10"], ["token", "question", "wing"]],
  "post" : [["settoken", "question", "0"]],
  "msg" : ["Where is it then?"]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/WyvernWing", "10"]],
  "post" : [["settoken", "question", "wing"]],
  "msg" : ["Have you returned with the wing of Slithselest?"],
  "replies": [["no", "Not yet, I still seek the foul beast's wing"], ["yes", "Yes, I return with the delicacy His Majesty seeks"]]
  },{
  "match" : ["no"],
  "pre" : [],
  "post" : [],
  "msg" : ["Very well then Lord $you, do return here if you change your mind."]
  },{
  "match" : ["yes", "service", "Scorn"],
  "pre" : [],
  "post" : [["quest", "scorn/WyvernWing", "10"]],
  "msg" : ["His Majesty, the King of Scorn, craves a snack.\n You will go and return with a wing from His Majesty's enemy, Slithselest, the self-styled Lord of Wyverns.\n When His Majesty dines upon this delicacy, you shall earn your Earldom.\n Slithselest has his eyrie (it is said) in the high mountains south of the road to Brest."]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hail Lord $you, would you be prepared to be of service to Scorn once again?"],
  "replies": [["no", "Not right now"], ["yes", "Of course, I would be delighted to serve this great city"]]
  }
]}