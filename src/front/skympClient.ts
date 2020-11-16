import {
  on,
  once,
  printConsole,
  storage,
  settings,
  Game,
  Ui,
  Utility,
  Actor,
} from "skyrimPlatform";
import { WorldView } from "./view";
import { getMovement } from "./components/movement";
import { getLook } from "./components/look";
import { AnimationSource, Animation, setupHooks } from "./components/animation";
import { getEquipment } from "./components/equipment";
import { getDiff, getInventory, Inventory } from "./components/inventory";
import { MsgType, HostStartMessage, HostStopMessage } from "./messages";
import { MsgHandler } from "./msgHandler";
import { ModelSource } from "./modelSource";
import { RemoteServer, getPcInventory } from "./remoteServer";
import { SendTarget } from "./sendTarget";
import * as networking from "./networking";
import * as sp from "skyrimPlatform";
import * as loadGameManager from "./loadGameManager";
import * as deathSystem from "./deathSystem";
import { setUpConsoleCommands } from "./console";
import { nextHostAttempt } from "./hostAttempts";

interface AnyMessage {
  type?: string;
  t?: number;
}
const handleMessage = (msgAny: AnyMessage, handler_: MsgHandler) => {
  const msgType: string = msgAny.type || MsgType[msgAny.t];
  const handler = (handler_ as unknown) as Record<
    string,
    (m: AnyMessage) => void
  >;
  const f = handler[msgType];
  /*if (msgType !== "UpdateMovement") {
    printConsole();
    for (const key in msgAny) {
      const v = (msgAny as Record<string, any>)[key];
      printConsole(`${key}=${JSON.stringify(v)}`);
    }
  }*/

  if (msgType === "hostStart") {
    const msg = msgAny as HostStartMessage;
    const target = msg.target;
    printConsole("hostStart", target.toString(16));

    let hosted = storage["hosted"];
    if (typeof hosted !== typeof []) {
      // if you try to switch to Set checkout .concat usage.
      // concat compiles but doesn't work as expected
      hosted = new Array<number>();
      storage["hosted"] = hosted;
    }

    if (!hosted.includes(target)) {
      hosted.push(target);
    }
  }

  if (msgType === "hostStop") {
    const msg = msgAny as HostStopMessage;
    const target = msg.target;
    printConsole("hostStop", target.toString(16));

    const hosted = storage["hosted"] as Array<number>;
    if (typeof hosted === typeof []) {
      storage["hosted"] = hosted.filter((x) => x !== target);
    }
  }

  if (f && typeof f === "function") handler[msgType](msgAny);
};

for (let i = 0; i < 100; ++i) printConsole();
printConsole("Hello Multiplayer");
printConsole("settings:", settings["skymp5-client"]);

const targetIp = settings["skymp5-client"]["server-ip"];
const targetPort = settings["skymp5-client"]["server-port"];

if (storage.targetIp !== targetIp || storage.targetPort !== targetPort) {
  storage.targetIp = targetIp;
  storage.targetPort = targetPort;

  printConsole(`Connecting to ${storage.targetIp}:${storage.targetPort}`);
  networking.connect(targetIp, targetPort);
} else {
  printConsole("Reconnect is not required");
}

export class SkympClient {
  private localIdToRemoteId(localFormId: number): number {
    if (localFormId >= 0xff000000) {
      const view = this.getView();
      if (!view) return 0;
      localFormId = view.getRemoteRefrId(localFormId);
      if (!localFormId) return 0;
    }
    return localFormId;
  }

  private remoteIdToLocalId(remoteFormId: number): number {
    if (remoteFormId >= 0xff000000) {
      const view = this.getView();
      if (!view) return 0;
      remoteFormId = view.getLocalRefrId(remoteFormId);
      if (!remoteFormId) return 0;
    }
    return remoteFormId;
  }

  constructor() {
    this.resetView();
    this.resetRemoteServer();
    setupHooks();

    sp.printConsole("SkympClient ctor");

    networking.on("connectionFailed", () => {
      printConsole("Connection failed");
    });

    networking.on("connectionDenied", (err: string) => {
      printConsole("Connection denied: ", err);
    });

    networking.on("connectionAccepted", () => {
      this.msgHandler.handleConnectionAccepted();
    });

    networking.on("disconnect", () => {
      this.msgHandler.handleDisconnect();
    });

    networking.on("message", (msgAny: Record<string, unknown>) => {
      handleMessage(msgAny, this.msgHandler);
    });

    on("update", () => {
      if (!this.singlePlayer) {
        this.sendInputs();
      }
    });

    let lastInv: Inventory;

    once("update", () => {
      const send = (msg: Record<string, unknown>) => {
        this.sendTarget.send(msg, true);
      };
      const localIdToRemoteId = (localId: number) => {
        return this.localIdToRemoteId(localId);
      };
      setUpConsoleCommands(send, localIdToRemoteId);
    });

    on("activate", (e) => {
      lastInv = getInventory(Game.getPlayer());
      const caster = e.caster ? e.caster.getFormID() : 0;
      let target = e.target ? e.target.getFormID() : 0;

      if (caster !== 0x14) return;
      if (!target) return;

      target = this.localIdToRemoteId(target);
      if (!target) return printConsole("localIdToRemoteId returned 0");

      this.sendTarget.send(
        { t: MsgType.Activate, data: { caster, target } },
        true
      );
      printConsole("sendActi", { caster, target });
    });

    type FurnitureId = number;
    const furnitureStreak = new Map<FurnitureId, Inventory>();

    on("containerChanged", (e) => {
      const oldContainerId = e.oldContainer ? e.oldContainer.getFormID() : 0;
      const newContainerId = e.newContainer ? e.newContainer.getFormID() : 0;
      const baseObjId = e.baseObj ? e.baseObj.getFormID() : 0;
      if (oldContainerId !== 0x14 && newContainerId !== 0x14) return;

      const furnitureRef = Game.getPlayer().getFurnitureReference();
      if (!furnitureRef) return;

      const furrnitureId = furnitureRef.getFormID();

      if (oldContainerId === 0x14 && newContainerId === 0) {
        let craftInputObjects = furnitureStreak.get(furrnitureId);
        if (!craftInputObjects) {
          craftInputObjects = { entries: [] };
        }
        craftInputObjects.entries.push({
          baseId: baseObjId,
          count: e.numItems,
        });
        furnitureStreak.set(furrnitureId, craftInputObjects);
        printConsole(
          `Adding ${baseObjId.toString(16)} (${e.numItems}) to recipe`
        );
      } else if (oldContainerId === 0 && newContainerId === 0x14) {
        printConsole("Flushing recipe");
        const craftInputObjects = furnitureStreak.get(furrnitureId);
        if (craftInputObjects && craftInputObjects.entries.length) {
          furnitureStreak.delete(furrnitureId);
          const workbench = this.localIdToRemoteId(furrnitureId);
          if (!workbench) return printConsole("localIdToRemoteId returned 0");

          this.sendTarget.send(
            {
              t: MsgType.CraftItem,
              data: { workbench, craftInputObjects, resultObjectId: baseObjId },
            },
            true
          );
          printConsole("sendCraft", {
            workbench,
            craftInputObjects,
            resultObjectId: baseObjId,
          });
        }
      }
    });

    on("containerChanged", (e) => {
      if (e.oldContainer && e.newContainer) {
        if (
          e.oldContainer.getFormID() === 0x14 ||
          e.newContainer.getFormID() === 0x14
        ) {
          printConsole(1);
          if (!lastInv) lastInv = getPcInventory();
          if (lastInv) {
            printConsole(2);
            const newInv = getInventory(Game.getPlayer());
            const diff = getDiff(lastInv, newInv, true);
            printConsole({ diff });
            diff.entries.forEach((entry) => {
              if (entry.count !== 0) {
                const msg = JSON.parse(JSON.stringify(entry));
                delete msg["name"]; // Extra name works too strange
                msg["t"] = entry.count > 0 ? MsgType.PutItem : MsgType.TakeItem;
                msg["count"] = Math.abs(msg["count"]);
                msg["target"] =
                  e.oldContainer.getFormID() === 0x14
                    ? e.newContainer.getFormID()
                    : e.oldContainer.getFormID();
                this.sendTarget.send(msg, true);
              }
            });
            lastInv = newInv;
          }
        }
      }
    });

    const playerFormId = 0x14;
    on("equip", (e) => {
      if (!e.actor || !e.baseObj) return;
      if (e.actor.getFormID() === playerFormId) {
        this.equipmentChanged = true;
        this.sendTarget.send(
          { t: MsgType.OnEquip, baseId: e.baseObj.getFormID() },
          false
        );
      }
    });
    on("unequip", (e) => {
      if (!e.actor || !e.baseObj) return;
      if (e.actor.getFormID() === playerFormId) {
        this.equipmentChanged = true;
      }
    });
    on("loadGame", () => {
      // Currently only armor is equipped after relogging (see remoteServer.ts)
      // This hack forces sending /equipment without weapons/ back to the server
      Utility.wait(3).then(() => (this.equipmentChanged = true));
    });

    loadGameManager.addLoadGameListener((e: loadGameManager.GameLoadEvent) => {
      if (!e.isCausedBySkyrimPlatform && !this.singlePlayer) {
        sp.Debug.messageBox(
          "Save has been loaded in multiplayer, switching to the single-player mode"
        );
        networking.close();
        this.singlePlayer = true;
        Game.setInChargen(false, false, false);
      }
    });
    on("update", () => deathSystem.update());
  }

  // May return null
  private getInputOwner(_refrId?: number) {
    return _refrId
      ? Actor.from(Game.getFormEx(this.remoteIdToLocalId(_refrId)))
      : Game.getPlayer();
  }

  private sendMovement(_refrId?: number) {
    const owner = this.getInputOwner(_refrId);
    if (!owner) return;

    const refrIdStr = `${_refrId}`;
    const sendMovementRateMs = 130;
    const now = Date.now();
    const last = this.lastSendMovementMoment.get(refrIdStr);
    if (!last || now - last > sendMovementRateMs) {
      this.sendTarget.send(
        {
          t: MsgType.UpdateMovement,
          data: getMovement(owner),
          _refrId,
        },
        false
      );
      this.lastSendMovementMoment.set(refrIdStr, now);
    }
  }

  private sendAnimation(_refrId?: number) {
    const owner = this.getInputOwner(_refrId);
    if (!owner) return;

    const refrIdStr = `${_refrId}`;

    let animSource = this.playerAnimSource.get(refrIdStr);
    if (!animSource) {
      animSource = new AnimationSource(owner);
      this.playerAnimSource.set(refrIdStr, animSource);
    }
    const anim = animSource.getAnimation();

    const lastAnimationSent = this.lastAnimationSent.get(refrIdStr);
    if (
      !lastAnimationSent ||
      anim.numChanges !== lastAnimationSent.numChanges
    ) {
      if (anim.animEventName !== "") {
        this.lastAnimationSent.set(refrIdStr, anim);
        this.sendTarget.send(
          { t: MsgType.UpdateAnimation, data: anim, _refrId },
          false
        );
      }
    }

    /*if (!this.playerAnimSource) {
      this.playerAnimSource = new AnimationSource(Game.getPlayer());
    }
    const anim = this.playerAnimSource.getAnimation();
    if (
      !this.lastAnimationSent ||
      anim.numChanges !== this.lastAnimationSent.numChanges
    ) {
      if (anim.animEventName !== "") {
        this.lastAnimationSent = anim;
        this.sendTarget.send(
          { t: MsgType.UpdateAnimation, data: anim, _refrId },
          false
        );
      }
    }*/
  }

  private sendLook(_refrId?: number) {
    if (_refrId) return;
    const shown = Ui.isMenuOpen("RaceSex Menu");
    if (shown != this.isRaceSexMenuShown) {
      this.isRaceSexMenuShown = shown;
      if (!shown) {
        printConsole("Exited from race menu");

        const look = getLook(Game.getPlayer());
        this.sendTarget.send(
          { t: MsgType.UpdateLook, data: look, _refrId },
          true
        );
      }
    }
  }

  private sendEquipment(_refrId?: number) {
    if (_refrId) return;
    if (this.equipmentChanged) {
      this.equipmentChanged = false;

      ++this.numEquipmentChanges;

      const eq = getEquipment(Game.getPlayer(), this.numEquipmentChanges);
      this.sendTarget.send(
        { t: MsgType.UpdateEquipment, data: eq, _refrId },
        true
      );
      printConsole({ eq });
    }
  }

  private sendHostAttempts() {
    const remoteId = nextHostAttempt();
    if (!remoteId) return;

    this.sendTarget.send({ t: MsgType.Host, remoteId }, false);
  }

  private sendInputs() {
    const hosted =
      typeof storage["hosted"] === typeof [] ? storage["hosted"] : [];
    const targets = [undefined].concat(hosted);
    //printConsole({ targets });
    targets.forEach((target) => {
      this.sendMovement(target);
      this.sendAnimation(target);
      this.sendLook(target);
      this.sendEquipment(target);
    });
    this.sendHostAttempts();
  }

  private resetRemoteServer() {
    const prevRemoteServer: RemoteServer = storage.remoteServer;
    let rs: RemoteServer;

    if (prevRemoteServer && prevRemoteServer.getWorldModel) {
      rs = prevRemoteServer;
      printConsole("Restore previous RemoteServer");

      // Keep previous RemoteServer, but update func implementations
      const newObj: Record<
        string,
        unknown
      > = (new RemoteServer() as unknown) as Record<string, unknown>;
      const rsAny: Record<string, unknown> = (rs as unknown) as Record<
        string,
        unknown
      >;
      for (const key in newObj) {
        if (typeof newObj[key] === "function") rsAny[key] = newObj[key];
      }
    } else {
      rs = new RemoteServer();
      printConsole("Creating RemoteServer");
    }

    this.sendTarget = rs;
    this.msgHandler = rs;
    this.modelSource = rs;
    storage.remoteServer = rs;
  }

  private resetView() {
    const prevView: WorldView = storage.view;
    const view = new WorldView();
    once("update", () => {
      if (prevView && prevView.destroy) {
        prevView.destroy();
        printConsole("Previous View destroyed");
      }
      storage.view = view;
    });
    on("update", () => {
      if (!this.singlePlayer) view.update(this.modelSource.getWorldModel());
    });
  }

  private getView(): WorldView | undefined {
    const res = storage.view as WorldView;
    if (typeof res === "object") return res;
    return undefined;
  }

  private playerAnimSource = new Map<string, AnimationSource>();
  private lastSendMovementMoment = new Map<string, number>();
  private lastAnimationSent = new Map<string, Animation>();
  private msgHandler?: MsgHandler;
  private modelSource?: ModelSource;
  private sendTarget?: SendTarget;
  private isRaceSexMenuShown = false;
  private singlePlayer = false;
  private equipmentChanged = false;
  private numEquipmentChanges = 0;
}

once("update", () => {
  // Is it racing with OnInit in Papyrus?
  sp.TESModPlatform.blockPapyrusEvents(true);
});
