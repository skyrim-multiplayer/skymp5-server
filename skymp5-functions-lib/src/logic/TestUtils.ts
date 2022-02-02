import { PlayerController } from "./PlayerController";

export const makePlayerController = (): PlayerController => {
  // TODO: use ES6 Proxy to automate handy methods adding
  // TODO: fix getName always returning undefined
  return {
    teleport: jest.fn(),
    setSpawnPoint: jest.fn(),
    showMessageBox: jest.fn(),
    sendChatMessage: jest.fn(),
    quitGame: jest.fn(),
    getName: jest.fn(x => { return `Player${x}`; }),
    addItem: jest.fn(),
    getRoundsArray: jest.fn().mockReturnValue([]),
    setRoundsArray: jest.fn(),
    getOnlinePlayers: jest.fn().mockReturnValue([1, 2]),
    setPercentages: jest.fn(),
    getPercentages: jest.fn(),
    getScriptName: jest.fn(),
    isTeleportActivator: jest.fn().mockReturnValue(true),
    updateCustomName: jest.fn(),
  };
};

export const resetMocks = (controller: PlayerController) => {
  const freshController = makePlayerController();
  for (const key in controller) {
    (controller as Record<string, unknown>)[key] = (freshController as Record<string, unknown>)[key];
  }
}
