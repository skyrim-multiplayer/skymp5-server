import { Inventory, LocationalData } from "../types/mp";
import { Position, squareDist } from "../utils/locationUtils";
import { GameModeListener } from "./GameModeListener";
import { Counter } from "./PlayerController";

export type TimedRewardController = {
  getCurrentTime(): Date;
  getOnlinePlayers(): number[];
  addItem(actorId: number, itemId: number, count: number): void;
  setCounter(actorId: number, counter: Counter, to: number): void;
  getCounter(actorId: number, counter: Counter): number;
  getInventory(actorId: number): Inventory;
  getLocation(actorId: number): LocationalData;
}

// export type Biome = 'pineForest' | 'aspenForest' | 'aaltoValley' | 'plains' | 'mountains' | 'tundra' | 'seaside';
export type BiomeName = string;

export type BiomeInfo = {
  name: BiomeName;
  pos: [number, number, number];
};

export type RewardRule = {
  itemFormId: number;
  itemCountWeights: number[];

  biome?: BiomeName;
  requiredItemFormId?: number;
}

// export type RewardRuleSet = Record<string, any[]/*RewardRuleSet*/> | {rule: RewardRule};

export type TimedRewardConfig = {
  // enableDaily, enablyHourly are here to simplify tests
  enableDaily: boolean;
  enableHourly: boolean;
  rules?: RewardRule[];
  // ruleSet?: RewardRuleSet;
  biomes?: BiomeInfo[];
}

export function getRandomIntByWeights(weights: number[]): number {
  let total = 0;
  for (const w of weights) {
    total += w;
  }
  let choice = Math.floor(Math.random() * total);
  for (let i = 0; i < weights.length; ++i) {
    if (choice < weights[i]) {
      return i;
    }
    choice -= weights[i];
  }
  return 0;
}

export function dayStart(date: Date): Date {
  const utcOffset = 1000 * 60 * 60 * 3; // 00:00 UTC+3
  const utcStart = new Date(date.getTime() + utcOffset);
  utcStart.setUTCHours(0, 0, 0, 0);
  return new Date(utcStart.getTime() - utcOffset);
}

export class SweetTaffyTimedRewards implements GameModeListener {
  static rewardItemFormId = 0x07F33922;

  constructor(private controller: TimedRewardController, private config: TimedRewardConfig) {
  }

  everySecond() {
    const currentTime = this.controller.getCurrentTime();
    const todayStart = dayStart(currentTime).getTime();
    for (const playerActorId of this.controller.getOnlinePlayers()) {
      if (this.config.enableDaily) {
        const lastDay = this.controller.getCounter(playerActorId, 'everydayStart');
        if (lastDay === 0) {
          this.controller.setCounter(playerActorId, 'everydayStart', todayStart);
          this.controller.setCounter(playerActorId, 'secondsToday', 0);
          this.controller.setCounter(playerActorId, 'lastExtraRewardDay', todayStart);
          this.controller.addItem(playerActorId, SweetTaffyTimedRewards.rewardItemFormId, 150);
        } else {
          const fullDays = Math.floor((currentTime.getTime() - lastDay) / (1000 * 60 * 60 * 24));
          if (fullDays > 0) {
            this.controller.setCounter(playerActorId, 'everydayStart', todayStart);
            this.controller.setCounter(playerActorId, 'secondsToday', 0);
            this.controller.addItem(playerActorId, SweetTaffyTimedRewards.rewardItemFormId, fullDays);
          }
        }
      }

      if (this.config.enableHourly) {
        const secondsToday = this.controller.getCounter(playerActorId, 'secondsToday') + 1;
        this.controller.setCounter(playerActorId, 'secondsToday', secondsToday);
        if (secondsToday == 60 * 60 && this.controller.getCounter(playerActorId, 'lastExtraRewardDay') !== todayStart) {
          this.controller.setCounter(playerActorId, 'lastExtraRewardDay', todayStart);
          this.giveExtraHourOfGameplayReward(playerActorId);
        }
      }
    }
  }

  private giveExtraHourOfGameplayReward(playerActorId: number) {
    this.controller.addItem(playerActorId, SweetTaffyTimedRewards.rewardItemFormId, 9);
    const playerBiome = this.getPlayerBiome(playerActorId);
    for (const rule of this.config.rules || []) {
      this.giveRewardByRule(playerActorId, playerBiome, rule);
    }
  }

  /*
  private giveExtraHourOfGameplayRewardWalk(playerActorId: number, ruleSet: RewardRuleSet) {
    const rule = ruleSet.rule;
    if (!Array.isArray(rule)) {
      this.giveRewardByRule(playerActorId, rule);
      return;
    }
    for (let [condition, subset] of rule) {
      ;
    }
  }
  */

  getPlayerBiome(playerActorId: number): BiomeName {
    const biomes = this.config.biomes || [];
    if (biomes.length === 0) {
      return '';
    }
    const {cellOrWorldDesc: playerCell, pos: playerPos} = this.controller.getLocation(playerActorId);
    if (playerCell !== '3c:Skyrim.esm') {
      return '';
    }
    const getSquareDist = (biomePos: Position) => squareDist(playerPos, biomePos);
    let closestBiomeIdx = 0, closestBiomeSquareDist = getSquareDist(biomes[0].pos);
    for (let i = 1; i < biomes.length; ++i) {
      const currentBiomeSquareDist = getSquareDist(biomes[i].pos);
      if (currentBiomeSquareDist < closestBiomeSquareDist) {
        closestBiomeIdx = i;
        closestBiomeSquareDist = currentBiomeSquareDist;
      }
    }
    return biomes[closestBiomeIdx].name;
  }

  playerHasItem(playerActorId: number, itemFormId: number) {
    for (const entry of this.controller.getInventory(playerActorId).entries) {
      if (entry.baseId === itemFormId) {
        return true;
      }
    }
    return false;
  }

  giveRewardByRule(playerActorId: number, playerBiome: BiomeName, rule: RewardRule) {
    if (rule.biome && rule.biome !== playerBiome) {
      return;
    }
    if (rule.requiredItemFormId && !this.playerHasItem(playerActorId, rule.requiredItemFormId)) {
      return;
    }
    const itemCount = getRandomIntByWeights(rule.itemCountWeights);
    if (itemCount === 0) {
      return;
    }
    this.controller.addItem(playerActorId, rule.itemFormId, itemCount);
  }
}
