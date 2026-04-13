function init(self)
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

    self.update = function(self, dt)
        if self.firstUpdateSinceActive then
            print("Entity script successfully updated!")
            self.firstUpdateSinceActive = false
        end
    end
end