function init(self)
    --TODO: Dialogue will be refactored into a component attached to the entity and have logic self-contained within its script
    self.dialogueOption = 1
    self.dialogueText = {
        "What a wonderful day it is outside",
        "I wonder how many time you can press this button",
        "Sometimes I think that I am sentient"
    }

    self.onCreated = function(self)
        print("ScriptComponent constructed! Do ctor stuff here... EntityID="..self.LOCAL_ENTITY_ID)
        self.firstUpdateSinceActive = true;
    end

    self.onDestroyed = function(self)
        print("ScriptComponent destroyed! Do cleanup stuff here...")
    end

    self.onActivate = function(self)
        print("Owning entity set to active! Initialise any functions and get ready to update!")
    end

    self.onDeactivate = function(self)
        print("Owning entity set to unactive! No longer need to update")
        self.firstUpdateSinceActive = true;
    end

    self.onPlayerInteract = function(self)
        if self.dialogueOption <= #self.dialogueText then
            Dialogue.Show()
            Dialogue.SetText(self.dialogueText[self.dialogueOption])
            self.dialogueOption = self.dialogueOption + 1
        else 
            Dialogue.Hide()
            self.dialogueOption = 1
        end

        -- TODO: I need to make a way to get the player ID from the scripts... 
        Entity.TurnToFace(self.LOCAL_ENTITY_ID, 1)
    end

    self.update = function(self, dt)
        if self.firstUpdateSinceActive then
            print("Entity script successfully updated!")
            self.firstUpdateSinceActive = false
        end
    end
end