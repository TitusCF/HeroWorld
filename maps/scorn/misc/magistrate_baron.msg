{
  "rules": [
  {
  "match" : ["mushroom", "cure", "blue", "yes"],
  "pre" : [["quest", "scorn/BlueMushroom", "10"], ["item", "blue mushroom"], ["token", "question", "mushroom"]],
  "post" : [["takeitem", "blue mushroom"], ["quest", "scorn/BlueMushroom", "50"], ["givecontents", "Marquis Reward"], ["settoken", "question", "0"]],
  "msg" : ["Congratulations on completing your quest.  Kneel and receive your accolade.....\n You are now a Marquis!\n Also, please take this fine potion as a reward. May it serve you well!"]
  },{
  "match" : ["mushroom", "cure", "blue", "yes"],
  "pre" : [["quest", "scorn/BlueMushroom", "10"], ["token", "question", "mushroom"]],
  "post" : [["settoken", "question", "0"]],
  "msg" : ["Where is it then?"]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/BlueMushroom", "10"]],
  "post" : [["settoken", "question", "mushroom"]],
  "msg" : ["Have you returned with the mushroom?"],
  "replies": [["no", "No, I still seek the mushroom"], ["yes", "Yes, I have here the cure that His Majesty desires"]]
  },{
  "match" : ["no"],
  "pre" : [],
  "post" : [],
  "msg" : ["Very well then Lord $you, do return here if you change your mind."]
  },{
  "match" : ["yes", "service", "Scorn"],
  "pre" : [],
  "post" : [["quest", "scorn/BlueMushroom", "10"]],
  "msg" : ["His Majesty, the King of Scorn, is suffering from bunions. \nHis honorable physician believes that a certain rare mushroom will provide a cure.\n The king pledges to elevate to the rank of Marquis whomever will provide this cure. \nThe mushrooms he needs are  blue in color, and grows only in the marshy areas located at the far west of the continent, same level as Scorn city."]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hail Lord $you, would you be prepared to be of service to Scorn once again?"],
  "replies": [["no", "Not right now"], ["yes", "Of course, I would be delighted to serve this great city"]]
  }
]}