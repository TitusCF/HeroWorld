{
  "rules": [
  {
  "match" : ["ring", "animator", "token", "yes"],
  "pre" : [["quest", "scorn/Animator", "10"], ["item", "Animator's Ring"], ["token", "question", "animator"]],
  "post" : [["takeitem", "Animator's Ring"], ["quest", "scorn/Animator", "50"], ["givecontents", "Baron Reward"], ["settoken", "question", "0"]],
  "msg" : ["Congratulations on completing your quest.  Kneel and receive your accolade.....\n You are now a Baron! \n Also, please take this fine longbow as a reward. May it serve you well!"]
  },{
  "match" : ["ring", "animator", "token", "yes"],
  "pre" : [["quest", "scorn/Animator", "10"], ["token", "question", "animator"]],
  "post" : [["settoken", "question", "0"]],
  "msg" : ["Where is it then?"]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/Animator", "10"]],
  "post" : [["settoken", "question", "animator"]],
  "msg" : ["Have you returned with a token of the Animator's destruction?"],
  "replies": [["no", "No, I still seek his destruction"], ["yes", "Yes, I have proof of his demise"]]
  },{
  "match" : ["no"],
  "pre" : [],
  "post" : [],
  "msg" : ["Very well then Sir $you, do return here if you change your mind."]
  },{
  "match" : ["yes", "service", "Scorn"],
  "pre" : [],
  "post" : [["quest", "scorn/Animator", "10"]],
  "msg" : ["His Majesty, the King of Scorn, is concerned with the outbreak of undead in the so-called 'Undead Church' in Scorn.  He suspects that an Animator is at work, setting the dead walking.\n\n He offers the title of Baron to whomever returns with a token proving that the Animator has been destroyed."]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hail Sir $you, would you be prepared to be of service to Scorn once again?"],
  "replies": [["no"], ["yes"]]
  }
]}