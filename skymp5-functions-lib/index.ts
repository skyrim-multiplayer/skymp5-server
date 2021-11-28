import { BrowserProperty } from './src/props/browserProperty';
import { ChatProperty } from './src/props/chatProperty';
import { DialogProperty } from './src/props/dialogProperty';
import { EvalProperty } from './src/props/evalProperty';
import { Ctx } from './src/types/ctx';
import { LocationalData, Mp } from './src/types/mp';
import { FunctionInfo } from './src/utils/functionInfo';
import { PersistentStorage } from './src/utils/persistentStorage';
import { Timer } from './src/utils/timer';

DialogProperty.init();
BrowserProperty.init();
EvalProperty.init();
ChatProperty.init();

Timer.init();

declare const mp: Mp;

const config = mp.getServerSettings();

console.log('gamemode.js reloaded');

const quitGamePortal = '42f3f:SweetPie.esp';
const neutralPortal = '42f70:SweetPie.esp';
const redPortal = '42e96:SweetPie.esp';
const bluePortal = '42fc1:SweetPie.esp';

type SpawnPoint = LocationalData;

interface SweetPieMap {
  // TODO: custom blue/red spawnpoints
  mainSpawnPoint: SpawnPoint;

  safePlace: SpawnPoint;
  safePlaceGoOutDoors: string[];
  safePlaceComeInDoors: string[];

  leaveMapDoors: string[];
}

class SweetPieRound {
  constructor(public readonly map: SweetPieMap) {}

  launchTimer = 60;
  maxPlayers = 20;
  players = new Set<number>();
  state: 'running' | 'warmup' = 'warmup';
}

const whiterun: SweetPieMap = {
  mainSpawnPoint: {
    pos: [22659, -8697, -3594],
    cellOrWorldDesc: '1a26f:Skyrim.esm',
    rot: [0, 0, 268],
  },
  safePlace: {
    pos: [-108, -809, 69.25],
    cellOrWorldDesc: '1605e:Skyrim.esm',
    rot: [0, 0, 176],
  },
  safePlaceGoOutDoors: ['16072:Skyrim.esm'],
  safePlaceComeInDoors: ['1a6f4:Skyrim.esm'],
  leaveMapDoors: ['1b1f3:Skyrim.esm'],
};

const rounds = new Array<SweetPieRound>();
rounds.push(new SweetPieRound(whiterun));

const findRoundForPlayer = (player: number): SweetPieRound | undefined => {
  return rounds.find((x) => x.state !== 'running');
};

const getPlayerCurrentRound = (player: number): SweetPieRound | undefined => {
  return rounds.find((x) => x.players.has(player));
};

const joinRound = (round: SweetPieRound, player: number) => {
  mp.set(player, 'locationalData', round.map.safePlace);
  round.players.add(player);
};

const leaveRound = (round: SweetPieRound | undefined, player: number) => {
  const hallSpawnPoint = {
    pos: [18511, 10256, 610.6392],
    cellOrWorldDesc: '42b5f:SweetPie.esp',
    rot: [0, 0, 347],
  };
  mp.set(player, 'locationalData', hallSpawnPoint);
  round?.players.delete(player);
};

const enum DialogId {
  NoAction,
  QuitGame,
  JoinDeathMatch,
  LeaveMatch,
}

mp.onActivate = (target: number, caster: number) => {
  const targetDesc = mp.getDescFromId(target);
  if (targetDesc === quitGamePortal) {
    DialogProperty.showMessageBox(caster, DialogId.QuitGame, 'quit game', 'going to leave sweetpie?', ['yes', 'no']);
    return false;
  }
  if (targetDesc === neutralPortal) {
    DialogProperty.showMessageBox(caster, DialogId.JoinDeathMatch, 'deathmatch', 'join deathmatch?', ['yes', 'no']);
    return false;
  }

  const lookupRes = mp.lookupEspmRecordById(target);
  const hasTeleport = lookupRes.record?.fields.findIndex((field) => field.type === 'XTEL') !== -1;
  if (hasTeleport) {
    const round = getPlayerCurrentRound(caster);
    if (round && round.map.safePlaceComeInDoors.indexOf(targetDesc) !== -1) {
      DialogProperty.showMessageBox(
        caster,
        DialogId.NoAction,
        'no way',
        'want to resign? leave the city. you can not hide in tavern',
        ['well']
      );
      return false;
    } else if (!round || round.map.leaveMapDoors.indexOf(targetDesc) !== -1) {
      DialogProperty.showMessageBox(caster, DialogId.LeaveMatch, 'run out', 'do you want to run out of town?', [
        'yes',
        'no',
      ]);
      return false;
    } else if (round.map.safePlaceGoOutDoors.indexOf(targetDesc) !== -1) {
      return true;
    } else {
      EvalProperty.eval(caster, (ctx) => {
        ctx.sp.Debug.notification('Interiors are not available during combat');
      });
      return false;
    }
  }

  return true;
};

DialogProperty.setDialogResponseHandler((response) => {
  switch (response.dialogId) {
    case DialogId.QuitGame:
      if (response.buttonIndex === 0) {
        EvalProperty.eval(response.actorId, (ctx: Ctx) => {
          ctx.sp.Game.quitToMainMenu();
          // TODO: close game
        });
      }
      break;
    case DialogId.JoinDeathMatch:
      if (response.buttonIndex === 0) {
        const round = findRoundForPlayer(response.actorId);
        if (round) {
          joinRound(round, response.actorId);
        } else {
          DialogProperty.showMessageBox(
            response.actorId,
            DialogId.NoAction,
            'no free maps',
            'sorry, there are no free slots for new players',
            ['well']
          );
        }
      }
      break;
    case DialogId.LeaveMatch:
      if (response.buttonIndex === 0) {
        const round = findRoundForPlayer(response.actorId);
        leaveRound(round, response.actorId);
      }
      break;
  }
  return true;
});

Timer.everySecond = () => {
  const onlinePlayer = mp.get(0, "onlinePlayers");
  onlinePlayer.forEach(actorId => {
    ChatProperty.showChat(actorId, true);
  });
};
