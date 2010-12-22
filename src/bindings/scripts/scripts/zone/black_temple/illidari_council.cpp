/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Illidari_Council
SD%Complete: 95
SDComment: Circle of Healing not working properly.
SDCategory: Black Temple
EndScriptData */

#include "precompiled.h"
#include "def_black_temple.h"

//Speech'n'Sounds
#define SAY_GATH_SLAY           -1564085
#define SAY_GATH_SLAY_COMNT     -1564089
#define SAY_GATH_DEATH          -1564093
#define SAY_GATH_SPECIAL1       -1564077
#define SAY_GATH_SPECIAL2       -1564081

#define SAY_VERA_SLAY           -1564086
#define SAY_VERA_COMNT          -1564089
#define SAY_VERA_DEATH          -1564094
#define SAY_VERA_SPECIAL1       -1564078
#define SAY_VERA_SPECIAL2       -1564082

#define SAY_MALA_SLAY           -1564087
#define SAY_MALA_COMNT          -1564090
#define SAY_MALA_DEATH          -1564095
#define SAY_MALA_SPECIAL1       -1564079
#define SAY_MALA_SPECIAL2       -1564083

#define SAY_ZERE_SLAY           -1564088
#define SAY_ZERE_COMNT          -1564091
#define SAY_ZERE_DEATH          -1564096
#define SAY_ZERE_SPECIAL1       -1564080
#define SAY_ZERE_SPECIAL2       -1564084

struct CouncilYells
{
    int32 entry;
    uint32 timer;
};

static CouncilYells CouncilAggro[]=
{
    {-1564069, 5000},                                       // Gathios
    {-1564070, 5500},                                       // Veras
    {-1564071, 5000},                                       // Malande
    {-1564072, 1200000},                                    // Zerevor
};

// Need to get proper timers for this later
static CouncilYells CouncilEnrage[]=
{
    {-1564073, 2000},                                       // Gathios
    {-1564074, 6000},                                       // Veras
    {-1564075, 5000},                                       // Malande
    {-1564076, 1200000},                                    // Zerevor
};

#define SPELL_BERSERK 45078

struct TRINITY_DLL_DECL mob_blood_elf_council_voice_triggerAI : public ScriptedAI
{
    mob_blood_elf_council_voice_triggerAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)me->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    uint64 m_council[4];

    uint32 m_enrageTimer;
    uint32 m_yellTimer;

    uint8 m_counter;                                      // Serves as the counter for both the aggro and enrage yells

    void Reset()
    {
        m_enrageTimer = 900000;                               // 15 minutes
        m_yellTimer = 500;

        m_counter = 0;
    }

    void AssignGUIDs()
    {
        m_council[0] = pInstance->GetData64(DATA_GATHIOSTHESHATTERER);
        m_council[1] = pInstance->GetData64(DATA_VERASDARKSHADOW);
        m_council[2] = pInstance->GetData64(DATA_LADYMALANDE);
        m_council[3] = pInstance->GetData64(DATA_HIGHNETHERMANCERZEREVOR);
    }

    void AttackStart(Unit *pWho){}

    void MoveInLineOfSight(Unit *pWho){}

    void UpdateAI(const uint32 diff)
    {
        if (pInstance->GetData(EVENT_ILLIDARICOUNCIL) != IN_PROGRESS)
            return;

        if (m_counter > 3)
            return;

        if (m_yellTimer < diff)
        {
            if (Unit *pMember = me->GetCreature(m_council[m_counter]))
            {
                DoScriptText(CouncilAggro[m_counter].entry, pMember);
                m_yellTimer = CouncilAggro[m_counter].timer;
            }

            m_counter += 1;
            if(m_counter > 3)
                m_counter = 0;                            // Reuse for Enrage Yells
        }
        else
            m_yellTimer -= diff;

        if (m_enrageTimer < diff)
        {
            if (Creature* pMember = pInstance->GetCreature(m_council[m_counter]))
            {
                pMember->CastSpell(pMember, SPELL_BERSERK, true);

                DoScriptText(CouncilEnrage[m_counter].entry, pMember);
                m_enrageTimer = CouncilEnrage[m_counter].timer;
            }

            m_counter += 1;
        }
        else
            m_enrageTimer -= diff;
    }
};

struct TRINITY_DLL_DECL mob_illidari_councilAI : public ScriptedAI
{
    mob_illidari_councilAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    uint64 m_council[4];

    uint32 m_checkTimer;
    uint32 m_endTimer;

    uint8 m_deathCount;

    void Reset()
    {
        m_checkTimer = 2000;

        m_endTimer = 0;
        m_deathCount = 0;

        for (uint8 i = 0; i < 4; ++i)
        {
            if(Creature* pMember = pInstance->GetCreature(m_council[i]))
            {
                if (!pMember->isAlive())
                {
                    pMember->RemoveCorpse();
                    pMember->Respawn();
                }
                pMember->AI()->EnterEvadeMode();
            }
        }

        pInstance->SetData(EVENT_ILLIDARICOUNCIL, NOT_STARTED);

        if(Creature *pTrigger = pInstance->GetCreature(pInstance->GetData64(DATA_BLOOD_ELF_COUNCIL_VOICE)))
            pTrigger->AI()->EnterEvadeMode();

        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetDisplayId(11686);
    }

    void EnterCombat(Unit *pWho){}
    void AttackStart(Unit *pWho) {}
    void MoveInLineOfSight(Unit *pWho){}

    void StartEvent(Unit *pTarget)
    {
        if (pTarget->isAlive())
        {
            m_council[0] = pInstance->GetData64(DATA_GATHIOSTHESHATTERER);
            m_council[1] = pInstance->GetData64(DATA_HIGHNETHERMANCERZEREVOR);
            m_council[2] = pInstance->GetData64(DATA_LADYMALANDE);
            m_council[3] = pInstance->GetData64(DATA_VERASDARKSHADOW);

            // Start the event for the Voice Trigger
            if (Creature *pTrigger = pInstance->GetCreature(pInstance->GetData64(DATA_BLOOD_ELF_COUNCIL_VOICE)))
                ((mob_blood_elf_council_voice_triggerAI*)pTrigger->AI())->AssignGUIDs();

            for (uint8 i = 0; i < 4; ++i)
            {
                if (m_council[i])
                {
                    if (Unit *pMember = pInstance->GetCreature(m_council[i]))
                    {
                        if (pMember->isAlive())
                            ((Creature*)pMember)->AI()->AttackStart(pTarget);
                    }
                }
            }

            pInstance->SetData(EVENT_ILLIDARICOUNCIL, IN_PROGRESS);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (pInstance->GetData(EVENT_ILLIDARICOUNCIL) != IN_PROGRESS)
            return;

        if (m_endTimer)
        {
            if (m_endTimer < diff)
            {
                if (m_deathCount > 3)
                {
                    if (Creature *pTrigger = pInstance->GetCreature(pInstance->GetData64(DATA_BLOOD_ELF_COUNCIL_VOICE)))
                        pTrigger->DealDamage(pTrigger,pTrigger->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);

                    pInstance->SetData(EVENT_ILLIDARICOUNCIL, DONE);

                    m_creature->DealDamage(m_creature, m_creature->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                    return;
                }

                if (Creature *pMember = pInstance->GetCreature(m_council[m_deathCount]))
                {
                    if (pMember->isAlive())
                        pMember->DealDamage(pMember, pMember->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                }

                ++m_deathCount;
                m_endTimer = 1500;
            }
            else
                m_endTimer -= diff;
        }

        if (m_checkTimer)
        {
            if (m_checkTimer < diff)
            {
                uint8 m_evadeCheck = 0;
                for (uint8 i = 0; i < 4; ++i)
                {
                    if (Creature *pMember = pInstance->GetCreature(m_council[i]))
                    {
                        // This is the evade/death check.
                        if (pMember->isAlive() && !pMember->getVictim())
                        {
                            m_evadeCheck += 1;
                            if (m_evadeCheck > 3)
                                Reset();
                        }
                        else
                        {
                            if (!pMember->isAlive())         // If even one member dies, kill the rest, set instance data, and kill self.
                            {
                                m_endTimer = 1000;
                                m_checkTimer = 0;
                                return;
                            }
                        }
                    }
                }
                m_checkTimer = 2000;
            }
            else
                m_checkTimer -= diff;
        }
    }
};

struct TRINITY_DLL_DECL illidari_council_baseAI : public ScriptedAI
{
    illidari_council_baseAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        loadedGUIDs = false;
    }

    uint64 m_council[4];

    uint32 m_checkTimer;

    ScriptedInstance* pInstance;

    bool loadedGUIDs;

    void EnterCombat(Unit *pWho)
    {
        if (Creature *pController = pInstance->GetCreature(pInstance->GetData64(DATA_ILLIDARICOUNCIL)))
            ((mob_illidari_councilAI*)pController->AI())->StartEvent(pWho);

        DoZoneInCombat();

        if (!loadedGUIDs)
        {
            m_council[0] = pInstance->GetData64(DATA_LADYMALANDE);
            m_council[1] = pInstance->GetData64(DATA_HIGHNETHERMANCERZEREVOR);
            m_council[2] = pInstance->GetData64(DATA_GATHIOSTHESHATTERER);
            m_council[3] = pInstance->GetData64(DATA_VERASDARKSHADOW);
            loadedGUIDs = true;
        }
    }

    void KilledUnit(Unit *pVictim)
    {
        switch (me->GetEntry())
        {
            case 22949: DoScriptText(SAY_GATH_SLAY, m_creature); break; // Gathios
            case 22950: DoScriptText(SAY_ZERE_SLAY, m_creature); break; // Zerevor
            case 22951: DoScriptText(SAY_MALA_SLAY, m_creature); break; // Melande
            case 22952: DoScriptText(SAY_VERA_SLAY, m_creature); break; // Veras
        }
    }

    void JustDied(Unit *pVictim)
    {
        switch (me->GetEntry())
        {
            case 22949: DoScriptText(SAY_GATH_DEATH, m_creature); break; // Gathios
            case 22950: DoScriptText(SAY_ZERE_DEATH, m_creature); break; // Zerevor
            case 22951: DoScriptText(SAY_MALA_DEATH, m_creature); break; // Melande
            case 22952: DoScriptText(SAY_VERA_DEATH, m_creature); break; // Veras
        }
    }

    void EnterEvadeMode()
    {
        for (uint8 i = 0; i < 4; ++i)
        {
            if (Creature *pTemp = pInstance->GetCreature(m_council[i]))
            {
                if (pTemp != me && pTemp->getVictim())
                {
                    AttackStart(pTemp->getVictim());
                    return;
                }
            }
        }
        ScriptedAI::EnterEvadeMode();
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        if(done_by == m_creature)
            return;

        damage /= 4;
        for (uint8 i = 0; i < 4; ++i)
        {
            if (Creature *pUnit = pInstance->GetCreature(m_council[i]))
            {
                if (pUnit != m_creature && pUnit->isAlive())
                {
                    pUnit->LowerPlayerDamageReq(damage);

                    if (damage <= pUnit->GetHealth())
                        pUnit->SetHealth(pUnit->GetHealth() - damage);
                    else
                        pUnit->Kill(pUnit, false);
                }
            }
        }
    }

};

// Gathios the Shatterer's spells
enum gathiosSpells
{
    SPELL_BLESS_PROTECTION     = 41450,
    SPELL_BLESS_SPELLWARD      = 41451,
    SPELL_CONSECRATION         = 41541,
    SPELL_HAMMER_OF_JUSTICE    = 41468,
    SPELL_SEAL_OF_COMMAND      = 41469,
    SPELL_SEAL_OF_BLOOD        = 41459,
    SPELL_GATHIOS_JUDGEMENT    = 41467,
    SPELL_CHROMATIC_AURA       = 41453,
    SPELL_DEVOTION_AURA        = 41452
};

struct TRINITY_DLL_DECL boss_gathios_the_shattererAI : public illidari_council_baseAI
{
    boss_gathios_the_shattererAI(Creature *c) : illidari_council_baseAI(c){}

    uint32 m_sealTimer;
    uint32 m_auraTimer;
    uint32 m_hammerTimer;
    uint32 m_blessingTimer;
    uint32 m_judgementTimer;
    uint32 m_consecrationTimer;

    uint32 m_checkTimer;

    void Reset()
    {
        m_consecrationTimer = 40000;
        m_hammerTimer = 10000;
        m_sealTimer = 40000;
        m_auraTimer = 90000;
        m_blessingTimer = 60000;
        m_judgementTimer = 45000;

        m_checkTimer = 1000;
    }

    Unit* SelectCouncil()
    {
        if(urand(0, 8))
        {
            if (Unit *pMelande = pInstance->GetCreature(m_council[0]))
            {
                if (pMelande->isAlive())
                    return pMelande;
            }
        }

        if (Unit *pCouncil = pInstance->GetCreature(m_council[urand(1,3)]))
        {
            if (pCouncil->isAlive())
                return pCouncil;
        }

        return me;
    }

    void ApplyAura(uint32 m_spellId)
    {
        for (uint8 i = 0; i < 4; ++i)
        {
            if (Unit *pCouncil = pInstance->GetCreature(m_council[i]))
                pCouncil->CastSpell(pCouncil, m_spellId, true, 0, 0, me->GetGUID());
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_checkTimer < diff)
        {
            DoZoneInCombat();
            m_checkTimer = 1000;
        }
        else
            m_checkTimer -= diff;

        if (m_blessingTimer < diff)
        {
            if(Unit *pUnit = SelectCouncil())
            {
                AddSpellToCast(pUnit, RAND(SPELL_BLESS_SPELLWARD, SPELL_BLESS_PROTECTION));
                m_blessingTimer = 15000;
            }
        }
        else
            m_blessingTimer -= diff;

        if (m_consecrationTimer < diff)
        {
            AddSpellToCast(m_creature, SPELL_CONSECRATION);
            m_consecrationTimer = 30000;
        }
        else
            m_consecrationTimer -= diff;

        if (m_hammerTimer < diff)
        {
            if(Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 40, true, 0, 10.0f))
            {
                AddSpellToCast(pTarget, SPELL_HAMMER_OF_JUSTICE);
                m_hammerTimer = 20000;
            }
        }
        else
            m_hammerTimer -= diff;

        if (m_sealTimer < diff)
        {
            AddSpellToCast(m_creature, RAND(SPELL_SEAL_OF_COMMAND, SPELL_SEAL_OF_BLOOD));
            m_sealTimer = 40000;
        }
        else
            m_sealTimer -= diff;

        if (m_judgementTimer < diff)
        {
            AddSpellToCast(me->getVictim(), SPELL_GATHIOS_JUDGEMENT);
            m_judgementTimer = urand(15000, 35000);
        }
        else
            m_judgementTimer -= diff;

        if (m_auraTimer < diff)
        {
            ApplyAura(RAND(SPELL_DEVOTION_AURA, SPELL_CHROMATIC_AURA));
            m_auraTimer = 90000;
        }
        else
            m_auraTimer -= diff;

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

// High Nethermancer Zerevor's spells
enum zerevorSpells
{
    SPELL_FLAMESTRIKE       = 41481,
    SPELL_BLIZZARD          = 41482,
    SPELL_ARCANE_BOLT       = 41483,
    SPELL_ARCANE_EXPLOSION  = 41524,
    SPELL_DAMPEN_MAGIC      = 41478
};

struct TRINITY_DLL_DECL boss_high_nethermancer_zerevorAI : public illidari_council_baseAI
{
    boss_high_nethermancer_zerevorAI(Creature *c) : illidari_council_baseAI(c){}

    uint32 m_blizzardTimer;
    uint32 m_flamestrikeTimer;
    uint32 m_dampenTimer;
    uint32 m_aexpTimer;
    uint32 m_boltTimer;

    void Reset()
    {
        m_blizzardTimer = urand(30000, 91000);
        m_flamestrikeTimer = urand(30000, 91000);
        m_dampenTimer = 2000;
        m_aexpTimer = 14000;
        m_boltTimer = 1000;

        m_checkTimer = 1000;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_checkTimer < diff)
        {
            if (me->GetDistance2d(me->getVictim()) < 20.0f)
                me->StopMoving();

            DoZoneInCombat();
            m_checkTimer = 1000;
        }
        else
            m_checkTimer -= diff;

        if (m_dampenTimer < diff)
        {
            ForceSpellCast(m_creature, SPELL_DAMPEN_MAGIC);
            m_dampenTimer = 67200;                      // almost 1,12 minutes
        }
        else
            m_dampenTimer -= diff;

        if (m_blizzardTimer < diff)
        {
            if(Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
            {
                AddSpellToCast(pTarget, SPELL_BLIZZARD);
                m_blizzardTimer = urand(45000, 90000);
            }
        }
        else
            m_blizzardTimer -= diff;

        if (m_flamestrikeTimer < diff)
        {
            if(Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true))
            {
                AddSpellToCast(pTarget, SPELL_FLAMESTRIKE);
                m_flamestrikeTimer = urand(55000, 100000);
            }
        }
        else
            m_flamestrikeTimer -= diff;

        if (m_boltTimer < diff)
        {
            if (!m_creature->IsNonMeleeSpellCasted(false))
                AddSpellToCast(m_creature->getVictim(), SPELL_ARCANE_BOLT);

            m_boltTimer = 1000;
        }
        else
            m_boltTimer -= diff;

        if (m_aexpTimer < diff)
        {
            std::list<HostilReference*>& m_threatlist = m_creature->getThreatManager().getThreatList();
            for (std::list<HostilReference*>::iterator i = m_threatlist.begin(); i!= m_threatlist.end();++i)
            {
                if (Unit *pUnit = pInstance->GetCreature((*i)->getUnitGuid()))
                {
                    if (pUnit->IsWithinDistInMap(me, 5))
                    {
                        ForceAOESpellCast(SPELL_ARCANE_EXPLOSION, INTERRUPT_AND_CAST);
                        break;
                    }
                }
            }
            m_aexpTimer = 1000;
        }
        else
            m_aexpTimer -= diff;

        CastNextSpellIfAnyAndReady();
    }
};

// Lady Malande's spells
enum malandeSpells
{
    SPELL_EMPOWERED_SMITE   = 41471,
    SPELL_CIRCLE_OF_HEALING = 41455,
    SPELL_REFLECTIVE_SHIELD = 41475,
    SPELL_DIVINE_WRATH      = 41472,
    SPELL_HEAL_VISUAL       = 24171
};

struct TRINITY_DLL_DECL boss_lady_malandeAI : public illidari_council_baseAI
{
    boss_lady_malandeAI(Creature *c) : illidari_council_baseAI(c){}

    uint32 m_smiteTimer;
    uint32 m_cohTimer;
    uint32 m_wrathTimer;
    uint32 m_shieldTimer;

    void Reset()
    {
        m_smiteTimer = 18000;
        m_cohTimer = 20000;
        m_wrathTimer = 40000;
        m_shieldTimer = 15000;

        m_checkTimer = 1000;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_checkTimer < diff)
        {
            DoZoneInCombat();
            m_checkTimer = 1000;
        }
        else
            m_checkTimer -= diff;

        if (m_smiteTimer < diff)
        {
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
            {
                AddSpellToCast(pTarget, SPELL_EMPOWERED_SMITE);
                m_smiteTimer = 30000;
            }
        }
        else
            m_smiteTimer -= diff;

        if (m_cohTimer < diff)
        {
            AddSpellToCast(m_creature, SPELL_CIRCLE_OF_HEALING);
            m_cohTimer = 30000;
        }
        else
            m_cohTimer -= diff;

        if (m_wrathTimer < diff)
        {
            if(Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
            {
                AddSpellToCast(pTarget, SPELL_DIVINE_WRATH);
                m_wrathTimer = urand(20000, 40000);
            }
        }
        else
            m_wrathTimer -= diff;

        if (m_shieldTimer < diff)
        {
            AddSpellToCast(m_creature, SPELL_REFLECTIVE_SHIELD);
            m_shieldTimer = 45000;
        }
        else
            m_shieldTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

// Veras Darkshadow's spells
enum verasSpells
{
    SPELL_DEADLY_POISON   = 41485,
    SPELL_DEADLY_POISON_T = 41480,
    SPELL_ENVENOM         = 41487,
    SPELL_VANISH          = 41476
};

struct TRINITY_DLL_DECL boss_veras_darkshadowAI : public illidari_council_baseAI
{
    boss_veras_darkshadowAI(Creature *c) : illidari_council_baseAI(c){}

    uint64 m_envenomGUID;

    uint32 m_poisonTimer;
    uint32 m_vanishTimer;
    uint32 m_envenomTimer;

    void Reset()
    {
        m_envenomGUID = 0;

        m_poisonTimer = 20000;
        m_vanishTimer = 10000;
        m_envenomTimer = 3000;

        m_checkTimer = 1000;

        m_creature->SetVisibility(VISIBILITY_ON);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void SpellHitTarget(Unit *pTarget, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_DEADLY_POISON)
        {
            m_envenomGUID = pTarget->GetGUID();
            DoStartMovement(pTarget);
            m_envenomTimer = 3600;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_checkTimer < diff)
        {
            DoZoneInCombat();
            m_checkTimer = 1000;
        }
        else
            m_checkTimer -= diff;

        if (me->GetVisibility() == VISIBILITY_ON)
        {
            if (m_vanishTimer < diff)
            {
                DoResetThreat();

                ForceSpellCast(me, SPELL_DEADLY_POISON_T, INTERRUPT_AND_CAST_INSTANTLY);
                ForceSpellCast(me, SPELL_VANISH, INTERRUPT_AND_CAST_INSTANTLY);
                m_vanishTimer = 30000;

                if(Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
                    DoStartMovement(pTarget);

                me->SetVisibility(VISIBILITY_OFF);
            }
            else
                m_vanishTimer -= diff;

            DoMeleeAttackIfReady();
        }
        else
        {
            if (m_vanishTimer < diff)
            {
                me->SetVisibility(VISIBILITY_ON);
                m_vanishTimer = 55000;
                return;
            }
            else
                m_vanishTimer -= diff;

            if (m_envenomTimer < diff)
            {
                if (m_envenomGUID)
                {
                    if(Unit *pTarget = me->GetUnit(m_envenomGUID))
                    {
                        AddSpellToCast(pTarget, SPELL_ENVENOM);
                        m_envenomGUID = 0;
                    }
                }
                m_envenomTimer = 3000;
            }
            else
                m_envenomTimer -= diff;

            CastNextSpellIfAnyAndReady();
        }
    }
};

CreatureAI* GetAI_mob_blood_elf_council_voice_trigger(Creature* c)
{
    return new mob_blood_elf_council_voice_triggerAI(c);
}

CreatureAI* GetAI_mob_illidari_council(Creature *_Creature)
{
    return new mob_illidari_councilAI (_Creature);
}

CreatureAI* GetAI_boss_gathios_the_shatterer(Creature *_Creature)
{
    return new boss_gathios_the_shattererAI (_Creature);
}

CreatureAI* GetAI_boss_lady_malande(Creature *_Creature)
{
    return new boss_lady_malandeAI (_Creature);
}

CreatureAI* GetAI_boss_veras_darkshadow(Creature *_Creature)
{
    return new boss_veras_darkshadowAI (_Creature);
}

CreatureAI* GetAI_boss_high_nethermancer_zerevor(Creature *_Creature)
{
    return new boss_high_nethermancer_zerevorAI (_Creature);
}

void AddSC_boss_illidari_council()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "mob_illidari_council";
    newscript->GetAI = &GetAI_mob_illidari_council;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_blood_elf_council_voice_trigger";
    newscript->GetAI = &GetAI_mob_blood_elf_council_voice_trigger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_gathios_the_shatterer";
    newscript->GetAI = &GetAI_boss_gathios_the_shatterer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_lady_malande";
    newscript->GetAI = &GetAI_boss_lady_malande;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_veras_darkshadow";
    newscript->GetAI = &GetAI_boss_veras_darkshadow;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_high_nethermancer_zerevor";
    newscript->GetAI = &GetAI_boss_high_nethermancer_zerevor;
    newscript->RegisterSelf();
}
