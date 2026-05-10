function init(self)
    self.tvEntityID = -1
    self.laptopEntityID = -1

    self.onActivate = function(self)
        self.tvEntityID = Entity.Create(SimpleVector.new(5, 3), SimpleVector.new(2, 1))
        print("TV Entity ID: "..self.tvEntityID)
        Entity.SetActive(self.tvEntityID, true)
        Entity.AddDialogueComponent(self.tvEntityID, Utility.Hash("PLAYER_BEDROOM_TV"), true, 10)

        self.laptopEntityID = Entity.Create(SimpleVector.new(9, 6), SimpleVector.new(1, 1))
        print("Laptop Entity ID: "..self.laptopEntityID)
        Entity.SetActive(self.laptopEntityID, true)
        Entity.AddDialogueComponent(self.laptopEntityID, Utility.Hash("PLAYER_BEDROOM_LAPTOP"), true, 0)
    end

    self.onDeactivate = function(self)
        Entity.Destroy(self.tvEntityID)
        Entity.Destroy(self.laptopEntityID)
    end

    self.update = function(self, dt)
    end
end
