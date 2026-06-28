const DATA = {
  "productions": [
    {"id":0,"lhs":"S'","rhs":["S"]},
    {"id":1,"lhs":"S", "rhs":["L","=","R"]},
    {"id":2,"lhs":"S", "rhs":["R"]},
    {"id":3,"lhs":"L", "rhs":["*","R"]},
    {"id":4,"lhs":"L", "rhs":["id"]},
    {"id":5,"lhs":"R", "rhs":["L"]}
  ],
  "states": [
    {"id":0,  "items":["S' -> . S, $","S -> . L = R, $","S -> . R, $","L -> . * R, $","L -> . * R, =","L -> . id, $","L -> . id, =","R -> . L, $"]},
    {"id":1,  "items":["S -> L . = R, $","R -> L ., $"]},
    {"id":2,  "items":["S -> R ., $"]},
    {"id":3,  "items":["S' -> S ., $"]},
    {"id":4,  "items":["L -> . * R, $","L -> . * R, =","L -> * . R, $","L -> * . R, =","L -> . id, $","L -> . id, =","R -> . L, $","R -> . L, ="]},
    {"id":5,  "items":["L -> id ., $","L -> id ., ="]},
    {"id":6,  "items":["S -> L = . R, $","L -> . * R, $","L -> . id, $","R -> . L, $"]},
    {"id":7,  "items":["R -> L ., $","R -> L ., ="]},
    {"id":8,  "items":["L -> * R ., $","L -> * R ., ="]},
    {"id":9,  "items":["R -> L ., $"]},
    {"id":10, "items":["S -> L = R ., $"]},
    {"id":11, "items":["L -> . * R, $","L -> * . R, $","L -> . id, $","R -> . L, $"]},
    {"id":12, "items":["L -> id ., $"]},
    {"id":13, "items":["L -> * R ., $"]}
  ],
  "transitions": [
    {"from":0,  "to":4,  "label":"*"},
    {"from":0,  "to":1,  "label":"L"},
    {"from":0,  "to":2,  "label":"R"},
    {"from":0,  "to":3,  "label":"S"},
    {"from":0,  "to":5,  "label":"id"},
    {"from":1,  "to":6,  "label":"="},
    {"from":4,  "to":4,  "label":"*"},
    {"from":4,  "to":7,  "label":"L"},
    {"from":4,  "to":8,  "label":"R"},
    {"from":4,  "to":5,  "label":"id"},
    {"from":6,  "to":11, "label":"*"},
    {"from":6,  "to":9,  "label":"L"},
    {"from":6,  "to":10, "label":"R"},
    {"from":6,  "to":12, "label":"id"},
    {"from":11, "to":11, "label":"*"},
    {"from":11, "to":9,  "label":"L"},
    {"from":11, "to":13, "label":"R"},
    {"from":11, "to":12, "label":"id"}
  ],
  "actionTable": {
    "0":  {"*":"s4",  "id":"s5"},
    "1":  {"$":"r5",  "=":"s6"},
    "2":  {"$":"r2"},
    "3":  {"$":"acc"},
    "4":  {"*":"s4",  "id":"s5"},
    "5":  {"$":"r4",  "=":"r4"},
    "6":  {"*":"s11", "id":"s12"},
    "7":  {"$":"r5",  "=":"r5"},
    "8":  {"$":"r3",  "=":"r3"},
    "9":  {"$":"r5"},
    "10": {"$":"r1"},
    "11": {"*":"s11", "id":"s12"},
    "12": {"$":"r4"},
    "13": {"$":"r3"}
  },
  "gotoTable": {
    "0":  {"L":1,  "R":2,  "S":3},
    "4":  {"L":7,  "R":8},
    "6":  {"L":9,  "R":10},
    "11": {"L":9,  "R":13}
  },
  "terminals":    ["$","*","=","id"],
  "nonTerminals": ["L","R","S","S'"],
  "hasConflict":  false,
  "conflicts":    [],
  "parseResult": {
    "accepted": true,
    "errorMessage": "",
    "steps": [
      {"stateStack":[0],          "symbolStack":[],                "input":["id","=","*","id","$"],"action":"Shift id, go to state 5",       "actionType":"shift",  "reduceProduction":-1},
      {"stateStack":[0,5],        "symbolStack":["id"],            "input":["=","*","id","$"],     "action":"Reduce by: L -> id",             "actionType":"reduce", "reduceProduction":4},
      {"stateStack":[0,1],        "symbolStack":["L"],             "input":["=","*","id","$"],     "action":"Shift =, go to state 6",        "actionType":"shift",  "reduceProduction":-1},
      {"stateStack":[0,1,6],      "symbolStack":["L","="],         "input":["*","id","$"],         "action":"Shift *, go to state 11",       "actionType":"shift",  "reduceProduction":-1},
      {"stateStack":[0,1,6,11],   "symbolStack":["L","=","*"],     "input":["id","$"],             "action":"Shift id, go to state 12",      "actionType":"shift",  "reduceProduction":-1},
      {"stateStack":[0,1,6,11,12],"symbolStack":["L","=","*","id"],"input":["$"],                  "action":"Reduce by: L -> id",             "actionType":"reduce", "reduceProduction":4},
      {"stateStack":[0,1,6,11,9], "symbolStack":["L","=","*","L"], "input":["$"],                  "action":"Reduce by: R -> L",              "actionType":"reduce", "reduceProduction":5},
      {"stateStack":[0,1,6,11,13],"symbolStack":["L","=","*","R"], "input":["$"],                  "action":"Reduce by: L -> * R",            "actionType":"reduce", "reduceProduction":3},
      {"stateStack":[0,1,6,9],    "symbolStack":["L","=","L"],     "input":["$"],                  "action":"Reduce by: R -> L",              "actionType":"reduce", "reduceProduction":5},
      {"stateStack":[0,1,6,10],   "symbolStack":["L","=","R"],     "input":["$"],                  "action":"Reduce by: S -> L = R",          "actionType":"reduce", "reduceProduction":1},
      {"stateStack":[0,3],        "symbolStack":["S"],             "input":["$"],                  "action":"Accept! Input parsed successfully.","actionType":"accept","reduceProduction":-1}
    ]
  }
};

const FIRST_SETS = {
  "id":  ["id"],
  "*":   ["*"],
  "=":   ["="],
  "$":   ["$"],
  "L":   ["id","*"],
  "R":   ["id","*"],
  "S":   ["id","*"],
  "S'":  ["id","*"]
};
