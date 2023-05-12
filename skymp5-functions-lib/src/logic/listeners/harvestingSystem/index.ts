import { PlayerController } from '../../PlayerController';
import { GameModeListener } from '../GameModeListener';
import { EspmLookupResult, Mp, ServerSettings } from '../../../types/mp';
import { Ctx } from '../../../types/ctx';
import { EvalProperty } from '../../../props/evalProperty';
import { getPossesedSkills } from '../skillMenu/skillMenuLogic';

declare const mp: Mp;
declare const ctx: Ctx;

export class HarvestingSystem implements GameModeListener {
  constructor(private mp: Mp, private controller: PlayerController) {
    this.serverSettings = this.mp.getServerSettings();
  }

  private serverSettings: ServerSettings;

  private static uint8ToNumber(data: Uint8Array): number {
    const uint8Arr = new Uint8Array(data);
    // TODO: call toGlobalRecordId on received id, won't work with mod items
    return new Uint32Array(uint8Arr.buffer)[0];
  }

  private static getNumberField(lookup: EspmLookupResult | Partial<EspmLookupResult>, fieldName: string):number {
    if (!lookup.record || !lookup.toGlobalRecordId) return NaN;
    const fieldIndex = lookup.record.fields.findIndex(field => field.type === fieldName);
    if (fieldIndex === -1) return NaN;
    return lookup.toGlobalRecordId(HarvestingSystem.uint8ToNumber(lookup.record.fields[fieldIndex].data));
  }

  onPlayerActivateObject(
    casterActorId: number,
    targetObjectDesc: string,
    targetId: number
  ): 'continue' | 'blockActivation' {
    const baseId = HarvestingSystem.getNumberField(mp.lookupEspmRecordById(targetId), "NAME");
    if (!baseId) return 'continue';

    const ingredientId =  HarvestingSystem.getNumberField(mp.lookupEspmRecordById(baseId), 'PFIG')
    if (!ingredientId) return 'continue';

    const isJazbayGrapes = 0x0006ac4a === ingredientId;
    const isIngredientToFood = [0x4b0ba, 0x34d22].includes(ingredientId);

    const skillType = [];

    const lookupResIngredient = mp.lookupEspmRecordById(ingredientId);
    if (!lookupResIngredient.record || !lookupResIngredient.toGlobalRecordId) return 'continue';
    const kwdaIndex = lookupResIngredient.record.fields.findIndex((field) => field.type === 'KWDA');
    if (kwdaIndex === -1) return 'continue';
    const keywordsArray = lookupResIngredient.record.fields[kwdaIndex].data;
    const importantKeywords = [];
    for (let i = 0; i < keywordsArray.length / 4; i++) {
      const keywordId = lookupResIngredient.toGlobalRecordId(
        HarvestingSystem.uint8ToNumber(lookupResIngredient.record.fields[kwdaIndex].data.slice(i * 4, (i + 1) * 4))
      );
      const keywordRecord = mp.lookupEspmRecordById(keywordId).record;
      if (!keywordRecord) return 'continue';
      if (keywordRecord.editorId === 'VendorItemFood' || keywordRecord.editorId === 'VendorItemIngredient') {
        importantKeywords.push(keywordRecord.editorId);
      }
    }

    if (importantKeywords.includes('VendorItemFood') || isJazbayGrapes || isIngredientToFood) {
      skillType.push('farmer');
    }
    if ((importantKeywords.includes('VendorItemIngredient') && !isIngredientToFood) || isJazbayGrapes) {
      skillType.push('doctor');
    }
    if (isJazbayGrapes) {
      skillType.push('bee');
    }

    if (skillType.length === 0) return 'continue';
    EvalProperty.eval(casterActorId, () => {
      const animations = ['IdleActivatePickUpLow', 'IdleActivatePickUp'];
      ctx.sp.Debug.sendAnimationEvent(ctx.sp.Game.getPlayer(), animations[Math.random() > 0.5 ? 1 : 0]);
    });

    const { possessedSkills } = getPossesedSkills(casterActorId);
    // 0 level is student, 1 level is adept...
    let maxLevel = -1;
    skillType.forEach((skillName) => {
      if (skillName in possessedSkills) {
        maxLevel = Math.max(possessedSkills[skillName].level, maxLevel);
      }
    });
    if (ingredientId === 0x00064b3f) return 'blockActivation';
    const additionalItemsNumber = maxLevel + (Math.random() > 0.5 ? 1 : 0);
    setTimeout(() => this.controller.addItem(casterActorId, ingredientId, additionalItemsNumber), 1000);

    return 'blockActivation';
  }
}
