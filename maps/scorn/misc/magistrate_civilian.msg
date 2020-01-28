{
  "rules": [
  {
  "match" : ["head", "goblin", "chief", "yes"],
  "pre" : [["quest", "scorn/GoblinKing", "10"], ["item", "Head of the Goblin Chief"], ["token", "question", "goblin"]],
  "post" : [["takeitem", "Head of the Goblin Chief"], ["quest", "scorn/GoblinKing", "50"], ["givecontents", "Knight Reward"], ["settoken", "question", "0"]],
  "msg" : ["Congratulations on completing your quest.  Kneel and receive your accolade.....\n You are now sir $you, Knight of Scorn, with all the privileges pertaining thereto."]
  },{
  "match" : ["head", "goblin", "chief", "yes"],
  "pre" : [["quest", "scorn/GoblinKing", "10"], ["token", "question", "goblin"]],
  "post" : [["settoken", "question", "0"]],
  "msg" : ["Where is it then?"]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/GoblinKing", "10"]],
  "post" : [["settoken", "question", "goblin"]],
  "msg" : ["Have you returned with the Head of the Goblin Chief?"],
  "replies": [["no", "No, I yet seek it"], ["yes", "Indeed I have"]]
  },{
  "match" : ["no"],
  "pre" : [],
  "post" : [],
  "msg" : ["Very well then, do return here if you change your mind."]
  },{
  "match" : ["yes", "service", "Scorn"],
  "pre" : [],
  "post" : [["quest", "scorn/GoblinKing", "10"]],
  "msg" : ["His Majesty, the King of Scorn, has announced that whomsoever returns with the Head of the Goblin Chief shall be awarded a Knighthood.\n\nThese goblins, who are said to live in the to the north east of Scorn mountains, have been making a nuisance of themselves to the North East of Scorn.  Go and make them regret their disorderly conduct."]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hello Adventurer, would you like to be of service to Scorn?"],
  "replies": [["no", "Nay, that holds no interest for me"], ["yes", "Absolutely"]]
  }
]}
