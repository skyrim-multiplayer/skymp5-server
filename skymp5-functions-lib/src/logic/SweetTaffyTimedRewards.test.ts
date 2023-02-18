import { Inventory } from "../types/mp";
import { Counter } from "./PlayerController";
import { dayStart, getRandomIntByWeights, RewardRule, SweetTaffyTimedRewards } from "./SweetTaffyTimedRewards";

export const mockController = () => {
  const counters = new Map<string, number>();
  return {
    addItem: jest.fn(),
    getOnlinePlayers: jest.fn(),
    getCurrentTime: jest.fn(),
    getCounter(actorId: number, counter: Counter) {
      return counters.get(actorId + ':' + counter) ?? 0;
    },
    setCounter(actorId: number, counter: Counter, to: number) {
      counters.set(actorId + ':' + counter, to);
    },
    getInventory: jest.fn(),
    getLocation: jest.fn(),
  };
};

describe("SweetTaffyTimedRewards", () => {
  test("dayStart works correctly", () => {
    expect(dayStart(new Date('2022-12-26T20:12:34.567+0300')).getTime())
      .toEqual(new Date('2022-12-26T00:00:00.000+0300').getTime());
    expect(dayStart(new Date('2022-12-26T00:00:00.000+0300')).getTime())
      .toEqual(new Date('2022-12-26T00:00:00.000+0300').getTime());
  });

  test("daily rewards", () => {
    const controller = mockController();
    const listener = new SweetTaffyTimedRewards(controller, { enableDaily: true, enableHourly: false });

    controller.setCounter(1, 'secondsToday', 1337);
    controller.setCounter(2, 'secondsToday', 1337);

    controller.getCurrentTime.mockReturnValueOnce(new Date('2022-12-26T23:00:00.000+0300'));
    controller.getOnlinePlayers.mockReturnValue([1]);

    listener.everySecond();
    expect(controller.getCounter(1, 'everydayStart')).toEqual(new Date('2022-12-26T00:00:00.000+0300').getTime());
    expect(controller.getCounter(1, 'lastExtraRewardDay')).toEqual(new Date('2022-12-26T00:00:00.000+0300').getTime());
    expect(controller.getCounter(1, 'secondsToday')).toEqual(0);  // online time counter should have been reset
    expect(controller.getCounter(2, 'everydayStart')).toEqual(0);
    expect(controller.getCounter(2, 'lastExtraRewardDay')).toEqual(0);
    expect(controller.getCounter(2, 'secondsToday')).toEqual(1337);
    expect(controller.addItem).toBeCalledTimes(1);
    expect(controller.addItem).toBeCalledWith(1, SweetTaffyTimedRewards.rewardItemFormId, 150);

    controller.getCurrentTime.mockReturnValueOnce(new Date('2022-12-27T00:00:00.000+0300'));
    controller.getOnlinePlayers.mockReturnValue([1, 2]);
    controller.addItem.mockReset();
  
    listener.everySecond();
    expect(controller.getCounter(1, 'everydayStart')).toEqual(new Date('2022-12-27T00:00:00.000+0300').getTime());
    expect(controller.getCounter(1, 'lastExtraRewardDay')).toEqual(new Date('2022-12-26T00:00:00.000+0300').getTime());
    expect(controller.getCounter(2, 'everydayStart')).toEqual(new Date('2022-12-27T00:00:00.000+0300').getTime());
    expect(controller.getCounter(2, 'lastExtraRewardDay')).toEqual(new Date('2022-12-27T00:00:00.000+0300').getTime());
    expect(controller.getCounter(2, 'secondsToday')).toEqual(0);  // online time counter should have been reset
    expect(controller.addItem).toBeCalledTimes(2);
    expect(controller.addItem).toBeCalledWith(1, SweetTaffyTimedRewards.rewardItemFormId, 1);
    expect(controller.addItem).toBeCalledWith(2, SweetTaffyTimedRewards.rewardItemFormId, 150);

    controller.getCurrentTime.mockReturnValueOnce(new Date('2022-12-31T00:00:00.000+0300'));
    controller.getOnlinePlayers.mockReturnValue([1, 2]);
    controller.addItem.mockReset();
  
    listener.everySecond();
    expect(controller.getCounter(1, 'everydayStart')).toEqual(new Date('2022-12-31T00:00:00.000+0300').getTime());
    expect(controller.getCounter(1, 'lastExtraRewardDay')).toEqual(new Date('2022-12-26T00:00:00.000+0300').getTime());
    expect(controller.getCounter(2, 'everydayStart')).toEqual(new Date('2022-12-31T00:00:00.000+0300').getTime());
    expect(controller.getCounter(2, 'lastExtraRewardDay')).toEqual(new Date('2022-12-27T00:00:00.000+0300').getTime());
    expect(controller.addItem).toBeCalledTimes(2);
    expect(controller.addItem).toBeCalledWith(1, SweetTaffyTimedRewards.rewardItemFormId, 4);
    expect(controller.addItem).toBeCalledWith(2, SweetTaffyTimedRewards.rewardItemFormId, 4);

    controller.getCurrentTime.mockReturnValueOnce(new Date('2022-12-31T23:59:59.999+0300'));
    controller.addItem.mockReset();

    listener.everySecond();
    expect(controller.getCounter(1, 'everydayStart')).toEqual(new Date('2022-12-31T00:00:00.000+0300').getTime());
    expect(controller.getCounter(1, 'lastExtraRewardDay')).toEqual(new Date('2022-12-26T00:00:00.000+0300').getTime());
    expect(controller.getCounter(2, 'everydayStart')).toEqual(new Date('2022-12-31T00:00:00.000+0300').getTime());
    expect(controller.getCounter(2, 'lastExtraRewardDay')).toEqual(new Date('2022-12-27T00:00:00.000+0300').getTime());
    expect(controller.addItem).toBeCalledTimes(0);
  });

  test("hourly rewards", () => {
    const controller = mockController();
    const listener = new SweetTaffyTimedRewards(controller, { enableDaily: false, enableHourly: true });

    controller.getCurrentTime.mockReturnValue(new Date('2022-12-26T23:00:00.000+0300'));
    controller.getOnlinePlayers.mockReturnValue([1]);

    for (let i = 0; i < 60 * 60; ++i) {
      expect(controller.getCounter(1, 'secondsToday')).toEqual(i);
      expect(controller.getCounter(1, 'lastExtraRewardDay')).toEqual(0);
      expect(controller.addItem).toBeCalledTimes(0);
      listener.everySecond();
    }
    expect(controller.getCounter(1, 'secondsToday')).toEqual(60 * 60);
    expect(controller.addItem).toBeCalledWith(1, SweetTaffyTimedRewards.rewardItemFormId, 9);

    // should not reward if extra reward was already given today
    controller.setCounter(1, 'secondsToday', 60 * 60 - 1);
    controller.addItem.mockReset();

    listener.everySecond();
    expect(controller.getCounter(1, 'secondsToday')).toEqual(60 * 60);
    expect(controller.addItem).toBeCalledTimes(0);
  });

  test("getRandomIntByWeights", () => {
    expect(getRandomIntByWeights([1337, 0])).toEqual(0);
    expect(getRandomIntByWeights([0, 1])).toEqual(1);
    expect(getRandomIntByWeights([0, 0, 0, 0, 322])).toEqual(4);
  });

  test("giveRewardByRule", () => {
    const controller = mockController();
    const listener = new SweetTaffyTimedRewards(controller, { enableDaily: false, enableHourly: false });
    const playerActorId = 1;

    const rule: RewardRule = {
      itemFormId: 0xbeef,
      itemCountWeights: [0, 1],
    };

    controller.getInventory.mockReturnValue({ entries: [] });

    listener.giveRewardByRule(playerActorId, 'tundra', rule);
    expect(controller.addItem).toBeCalledTimes(1);
    expect(controller.addItem).toBeCalledWith(playerActorId, rule.itemFormId, 1);

    rule.requiredItemFormId = 0x1337;

    controller.addItem.mockClear();
    listener.giveRewardByRule(playerActorId, 'tundra', rule);
    expect(controller.addItem).toBeCalledTimes(0);

    const inventory: Inventory = {
      entries: [
        {
          baseId: rule.requiredItemFormId,
          count: 1,
        },
      ],
    };
    controller.getInventory.mockReturnValue(inventory);

    controller.addItem.mockClear();
    listener.giveRewardByRule(playerActorId, 'tundra', rule);
    expect(controller.addItem).toBeCalledTimes(1);
    expect(controller.addItem).toBeCalledWith(playerActorId, rule.itemFormId, 1);

    rule.biome = 'mountains';

    controller.addItem.mockClear();
    listener.giveRewardByRule(playerActorId, 'tundra', rule);
    expect(controller.addItem).toBeCalledTimes(0);

    controller.addItem.mockClear();
    listener.giveRewardByRule(playerActorId, 'mountains', rule);
    expect(controller.addItem).toBeCalledTimes(1);
    expect(controller.addItem).toBeCalledWith(playerActorId, rule.itemFormId, 1);
  });
});
