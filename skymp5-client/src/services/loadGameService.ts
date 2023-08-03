import EventEmitter from "eventemitter3";
import { ClientListener, CombinedController, Sp } from "./clientListener";

export interface GameLoadEvent {
    isCausedBySkyrimPlatform: boolean;
}

export class LoadGameService implements ClientListener {
    constructor(private sp: Sp, private controller: CombinedController) {
        this.controller.registerListenerForLookup("LoadGameService", this);
        this.controller.on("loadGame", () => this.onLoadGame());
    }

    get events() {
        return this._emitter;
    }

    private onLoadGame() {
        try {
            const gameLoadEvent: GameLoadEvent = {
                isCausedBySkyrimPlatform: this._isCausedBySkyrimPlatform
            };
            this._emitter.emit("gameLoad", gameLoadEvent);
        } catch (e) {
            this.controller.once("tick", () => {
                this._isCausedBySkyrimPlatform = false;
            });
            throw e;
        }
        this.controller.once("tick", () => {
            this._isCausedBySkyrimPlatform = false;
        });
    }

    private _isCausedBySkyrimPlatform = false;
    private _emitter = new EventEmitter<"gameLoad", GameLoadEvent>;
}
