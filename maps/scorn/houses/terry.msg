{
  "location" : "scorn_terrysfarm",
  "rules": [
  {
  "match" : ["*"],
  "pre" : [["quest", "scorn/TerrysFarm", "70"]],
  "post": [],
  "msg" : ["You killed my wife! Die Murderer!"]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/TerrysFarm", "60"]],
  "post": [],
  "msg" : ["Thank you for saving us from the bandits"]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/TerrysFarm", "30"]],
  "post": [],
  "msg" : ["Thank you for agreeing to save us from the bandits."]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/TerrysFarm", "20"]],
  "post": [],
  "msg" : ["June wouldn't give you the key because you aren't strong enough? I hope she knows what she is doing, those bandits won't stay locked up forever."]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/TerrysFarm", "10"]],
  "post": [],
  "msg" : ["My wife is over by the stove, speak to her and she might give you the key."]
  },{
  "match" : ["*"],
  "pre" : [],
  "post": [["quest", "scorn/TerrysFarm", "10"]],
  "msg" : ["The bandits have invaded our farm. They are killing our chickens and stealing our potatoes.\n We have them locked up for now, but we need to get them out of the farmhouse.\nPlease stop them! \nMy wife has the key"]
  }
]}