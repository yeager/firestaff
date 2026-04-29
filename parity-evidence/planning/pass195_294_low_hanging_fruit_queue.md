# Pass 195–294 — Low-hanging ReDMCSB source-lock queue

Rule: every pass begins in ReDMCSB source and must cite file/function/line evidence before probes/implementation.

## Input/routing table locks
195. Interface keyboard routes — `COMMAND.C:G0458_as_Graphic561_PrimaryKeyboardInput_Interface`.
196. Party resting input routes — `COMMAND.C:G0450_as_Graphic561_PrimaryMouseInput_PartyResting`.
197. Frozen game input routes — `COMMAND.C:G0451_as_Graphic561_PrimaryMouseInput_FrozenGame`.
198. Action area name routes — `COMMAND.C:G0452_as_Graphic561_MouseInput_ActionAreaNames`.
199. Action area icon routes — `COMMAND.C:G0453_as_Graphic561_MouseInput_ActionAreaIcons`.
200. Spell area symbol routes — `COMMAND.C:G0454_as_Graphic561_MouseInput_SpellArea`.
201. Champion names/hands route split — `COMMAND.C:G0455_as_Graphic561_MouseInput_ChampionNamesHands`.
202. Panel chest mouse routes — `COMMAND.C:G0456_as_Graphic561_MouseInput_PanelChest`.
203. Resurrect/reincarnate/cancel panel routes — `COMMAND.C:G0457...PanelResurrectReincarnateCancel`.
204. Rename mouse keyboard route family — `COMMAND.C` rename arrays.
205. Dialog viewport-vs-screen input set selector — `COMMAND.C:G0480_aaps_PrimaryMouseInput_DialogSets`.
206. Directional movement mouse boxes — `COMMAND.C` movement input zones.
207. Door/viewport C080 input gate — `COMMAND.C` C080 route + `CLIKVIEW.C` dispatch.
208. Panel C081 input gate — `COMMAND.C` C081 route + panel dispatch.
209. Freeze/unfreeze command route — `COMMAND.C` C147/C148.
210. Save-game keyboard route variants — `COMMAND.C:G0458`, C140.
211. Inventory F1-F4 keyboard variants — `COMMAND.C:G0458`, C007-C010.
212. Entrance resume/credits command distinction — `COMMAND.C`, `ENTRANCE.C`.
213. Quit key command variants — `COMMAND.C`, C216.
214. Dialog choice command arithmetic — `COMMAND.C:2459-2461`.

## HUD / champion / inventory
215. Champion status name zones — `COMMAND.C`, `COORD.C` C159-C162.
216. Champion status hand zones — `DATA.C:G0030`, `COORD.C` C211-C218.
217. Champion portrait boxes — `DATA.C:G0047`, `COORD.C`.
218. Mouth/eye boxes — `DATA.C:G0048/G0049`, `COMMAND.C` C545/C546.
219. Wound defense factor table — `DATA.C:G0050`.
220. Slot masks table — `DATA.C:G0038`.
221. Slot drop order — `DATA.C:G0057`.
222. Inventory chest slot storage route — `CHAMPION.C:F0302`.
223. Leader hand swap route — `CHAMPION.C:F0297/F0302`.
224. Inventory toggle/close source path — `INVENTORY.C:F0355`.
225. Inventory mouth consequence — `INVENTORY.C:F0349`.
226. Inventory eye consequence — `INVENTORY.C:F0352`.
227. Panel content enum route — `INVENTORY.C` panel content state.
228. Food/water/poison panel draw route — `INVENTORY.C` panel draw.
229. Scroll panel route — `INVENTORY.C` scroll hand panel.
230. Chest open/close route — `INVENTORY.C:F0334/F0335`.
231. Rename character map — `DATA.C:G2047`, `COMMAND.C` rename commands.
232. Champion rename keyboard map — `COMMAND.C` rename keyboard input.
233. Champion icon release route — `COMMAND.C:C129`, `CHAMPION.C`.
234. Champion action area consequence — `COMMAND.C:F0371`.

## Spell/action system
235. Magic caster set route — `COMMAND.C:C109`.
236. Spell rune 1 route — `COMMAND.C:C101`.
237. Spell rune 2 route — `COMMAND.C:C102`.
238. Spell rune 3 route — `COMMAND.C:C103`.
239. Spell rune 4 route — `COMMAND.C:C104`.
240. Spell rune 5 route — `COMMAND.C:C105`.
241. Spell rune 6 route — `COMMAND.C:C106`.
242. Spell recant route — `COMMAND.C:C107`.
243. Spell cast route — `COMMAND.C:C108`.
244. Click-in-spell-area gate — `COMMAND.C:C100/F0370`.
245. Click-in-action-area gate — `COMMAND.C:C111/F0371`.
246. Pass action route — `COMMAND.C:C112`.
247. Champion action 0 route — `COMMAND.C:C113`.
248. Champion action 1 route — `COMMAND.C:C114`.
249. Champion action 2 route — `COMMAND.C:C115`.
250. Champion action icon 0–3 route — `COMMAND.C:C116-C119`.

## Viewport/world visuals and clicks
251. C080 coordinate normalization — `CLIKVIEW.C`.
252. Front door button source route — `CLIKVIEW.C`, `DUNVIEW.C`.
253. Wall ornament click route — `CLIKVIEW.C`.
254. Fountain click route — `CLIKVIEW.C`.
255. Alcove/floor item grab route — `CLIKVIEW.C`.
256. Knock-on-wall route — `CLIKVIEW.C`.
257. Door button draw source — `DUNVIEW.C:F0110`.
258. Door state draw route — `DUNVIEW.C`.
259. Wall ornament draw route — `DUNVIEW.C`.
260. Floor item draw route — `DUNVIEW.C`.
261. Creature draw ordering route — `DUNVIEW.C`.
262. Projectile draw route — `DUNVIEW.C`.
263. Viewport palette/light route — `DUNVIEW.C`, `DATA.C`.
264. Wall set / decoration side route — `DUNVIEW.C`.
265. Side contents route — `DUNVIEW.C`.
266. Door animation runtime route — `DOOR.C`, `DUNVIEW.C`.
267. Sensor trigger route for click — `SENSOR.C`, `CLIKVIEW.C`.
268. Movement sensor route — `MOVESENS.C`.
269. Pit/stairs movement route — `MOVESENS.C`.
270. Champion portrait-on-wall route — `DUNVIEW.C`, `CLIKVIEW.C`.

## Frontends / lifecycle / dialogs
271. Title key/mouse transition route — `TITLE.C`, `COMMAND.C`.
272. Title palette/fade route — `TITLE.C`, `DATA.C`.
273. Entrance pre-draw microdungeon route — `ENTRANCE.C`.
274. Entrance credits route — `ENTRANCE.C:F0442`.
275. Entrance fade route — `ENTRANCE.C` palette.
276. Entrance pointer show/hide schedule — `ENTRANCE.C`.
277. Dialog graphic expansion path — `DIALOG.C:F0427`.
278. Dialog patch negative graphics route — `DIALOG.C`, `COORD.C`.
279. Dialog message split route — `DIALOG.C:F0426`.
280. Dialog choice click feedback route — `DIALOG.C:F0424`.
281. Dialog input table swap/restore route — `DIALOG.C:F0424`.
282. Restart screen draw boxes — `ENDGAME.C`, `DATA.C`.
283. Endgame THE END box route — `ENDGAME.C`, `DATA.C`.
284. Endgame champion summary route — `ENDGAME.C`.
285. Endgame credits wait/restart after credits — `ENDGAME.C`.
286. Endgame quit button route — `ENDGAME.C`, `COMMAND.C`.

## Data/audio/cache small locks
287. Credits/entrance special palette full rows — `DATA.C:G0019/G0020`.
288. VGA special palette mapping to compat table — `DATA.C`, existing palette code.
289. SND3 direct marker TODO bucket 1 — pass55 findings + sound source calls.
290. SND3 direct marker TODO bucket 2 — pass55 findings + sound source calls.
291. SND3 direct marker TODO bucket 3 — pass55 findings + sound source calls.
292. SND3 direct marker TODO bucket 4 — pass55 findings + sound source calls.
293. Graphics DAT local palette route — graphics DAT loader source.
294. Derived bitmap cache negative graphic route — cache/bitmap source.
