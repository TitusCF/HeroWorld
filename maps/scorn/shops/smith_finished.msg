{
  "rules": [
  {
  "match" : ["no"],
  "pre" : [["token", "completed", "eyeshield", "redshield", "redmail", "bluemail", "greenmail"]],
  "post": [["settoken", "completed", "0"]],
  "msg" : ["Ok then, I'll keep it here until you come back to collect it."]
  },

  { 
  "comment": "Collecting a previous order",
  "match" : ["yes"],
  "pre" : [["token", "completed", "eyeshield"]],
  "post": [["settoken", "completed", "0"], ["giveitem", "eyeshield"], ["settoken", "orderplaced", "0"], ["quest", "scorn/smith/customarmour", "40"], ["settoken", "ordered", "0"]],
  "msg" : ["Here is your eyeshield, $you"]
  },{
  "match" : ["yes"],
  "pre" : [["token", "completed", "redshield"]],
  "post": [["settoken", "completed", "0"], ["giveitem", "dragon shield"], ["settoken", "orderplaced", "0"], ["quest", "scorn/smith/customarmour", "40"],["settoken", "ordered", "0"]],
  "msg" : ["Here is your dragon shield, $you"]
  },{
  "match" : ["yes"],
  "pre" : [["token", "completed", "redmail"]],
  "post": [["settoken", "completed", "0"], ["giveitem", "red dragon mail"], ["settoken", "orderplaced", "0"], ["quest", "scorn/smith/customarmour", "40"],["settoken", "ordered", "0"]],
  "msg" : ["Here is your red dragon mail, $you"]
  },{
  "match" : ["yes"],
  "pre" : [["token", "completed", "bluemail"]],
  "post": [["settoken", "completed", "0"], ["giveitem", "blue dragon mail"], ["settoken", "orderplaced", "0"], ["quest", "scorn/smith/customarmour", "40"], ["settoken", "ordered", "0"]],
  "msg" : ["Here is your blue dragon mail, $you"]
  },{
  "match" : ["yes"],
  "pre" : [["token", "completed", "greenmail"]],
  "post": [["settoken", "completed", "0"], ["giveitem", "green dragon mail"], ["settoken", "orderplaced", "0"], ["quest", "scorn/smith/customarmour", "40"],["settoken", "ordered", "0"]],
  "msg" : ["Here is your green dragon mail, $you"]
  },


  { 
  "comment": "Previous Order is ready",
  "match" : ["*"],
  "pre" : [["token", "ordered", "greenmail"]],
  "post": [["settoken", "completed", "greenmail"]],
  "msg" : ["Hello $you, I've finished your green dragon mail, would you like to collect it now?"]
  },{
  "match" : ["*"],
  "pre" : [["token", "ordered", "eyeshield"]],
  "post": [["settoken", "completed", "eyeshield"]],
  "msg" : ["Hello $you, I've finished your Eyeshield, would you like to collect it now?"]
  },{
  "match" : ["*"],
  "pre" : [["token", "ordered", "bluemail"]],
  "post": [["settoken", "completed", "bluemail"]],
  "msg" : ["Hello $you, I've finished your blue dragon mail, would you like to collect it now?"]
  },{
  "match" : ["*"],
  "pre" : [["token", "ordered", "redmail"]],
  "post": [["settoken", "completed", "redmail"]],
  "msg" : ["Hello $you, I've finished your red dragon mail, would you like to collect it now?"]
  },{
  "match" : ["*"],
  "pre" : [["token", "ordered", "redshield"]],
  "post": [["settoken", "completed", "redshield"]],
  "msg" : ["Hello $you, I've finished your dragon shield, would you like to collect it now?"]
  },

  {
  "comment": "This should never ever happen, unless the server admin has been messing around",
  "match" : ["*"],
  "pre" : [],
  "post": [["settoken", "orderplaced", "0"]],
  "msg" : ["Hmm, that's very strange, I seem to have misplaced your order, maybe you should ask the a dungeon master if they can find where I put it?"]
  }
]}