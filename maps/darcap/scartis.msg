{
  "rules": [
  {
  "match" : ["wine"],
  "pre" : [["quest", "darcap/Cameron-wine", "=10"], ["npctoken", "greet", "1"]],
  "post" : [["quest", "darcap/Cameron-wine", "20"]],
  "msg" : ["Ha, if Cameron trusts you, then I guess I can trust you too.\n\nActually, my winemaker lost his special scythe. Without it, he can't cut grapes - so he says.\n\nI suggest you go check with him. The farm is located just west of Darcap's exit."],
  "replies" : []
  },{
  "match" : ["*"],
  "pre" : [["quest", "darcap/Cameron-wine", "=10"]],
  "post" : [["setnpctoken", "greet", "1"]],
  "msg" : ["Hello, and welcome to Scartis's Fine Wines!\n\nHere you will find wines that you can't find any place else, guaranteed!"],
  "replies" : [["wine", "Cameron, the tavern owner, is worried to not get your famous wine, lately...", "0"]]
  },{
  "match" : ["scythe"],
  "pre" : [["quest", "darcap/Cameron-wine", "=50"]],
  "post" : [["quest", "darcap/Cameron-wine", "60"]],
  "msg" : ["Ho, that's great!\n\nThank you very much, now I can again sell my famous wine!"],
  "replies" : []
  },{
  "match" : ["*"],
  "pre" : [["quest", "darcap/Cameron-wine", "=50"]],
  "post" : [],
  "msg" : ["Hello, and welcome to Scartis's Fine Wines!\n\nHere you will find wines that you can't find any place else, guaranteed!"],
  "replies" : [["scythe", "I brought back Talok's scythe, so she can again cut grapes.", "0"]]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hello, and welcome to Scartis's Fine Wines!\n\nHere you will find wines that you can't find any place else, guaranteed!"],
  "replies" : []
  }
]}
