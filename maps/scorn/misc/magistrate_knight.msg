{
  "rules": [
  {
  "match" : ["head", "ogre", "chief", "yes"],
  "pre" : [["quest", "scorn/OgreKing", "10"], ["item", "Head of the Ogre Chief"], ["token", "question", "ogre"]],
  "post" : [["takeitem", "Head of the Ogre Chief"], ["quest", "scorn/OgreKing", "50"], ["givecontents", "Baronet Reward"], ["settoken", "question", "0"]],
  "msg" : ["Congratulations on completing your quest.  Kneel and receive your accolade.....\n You are now a Baronet!\n Also, please take this girdle as a reward. May it serve you well!"]
  },{
  "match" : ["head", "ogre", "chief", "yes"],
  "pre" : [["quest", "scorn/OgreKing", "10"], ["token", "question", "ogre"]],
  "post" : [["settoken", "question", "0"]],
  "msg" : ["Where is it then?"]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/OgreKing", "10"]],
  "post" : [["settoken", "question", "ogre"]],
  "msg" : ["Have you returned with the Head of the Ogre Chief?"],
  "replies": [["no", "No, I still seek his head"], ["yes", "I have, and here it is."]]
  },{
  "match" : ["no"],
  "pre" : [],
  "post" : [],
  "msg" : ["Very well then Sir $you, do return here if you change your mind."]
  },{
  "match" : ["yes", "service", "Scorn"],
  "pre" : [],
  "post" : [["quest", "scorn/OgreKing", "10"]],
  "msg" : ["His Majesty, the King of Scorn, announces that whomsoever returns with the Head of the Ogre Chief shall be awarded a Baronetcy. \n\n His tribe of ogres have been stealing sheep from the hills east of the city. \n\n Go and decapitate the tribe. \n Our scouts have reported sightings of members of the tribe near a hole north of the road to Navar."]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hail Sir $you, would you be prepared to be of further service to Scorn?"],
  "replies": [["no", "Not right now"], ["yes", "Of course, I would be delighted to serve this great city"]]
  }
]}