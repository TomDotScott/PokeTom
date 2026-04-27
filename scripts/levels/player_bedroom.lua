function init(self)
    self.tvEntityID = -1
    self.laptopEntityID = -1

    self.onActivate = function(self)
        self.tvEntityID = Entity.Create(5 * 32, 3 * 32)
        print("TV Entity ID: "..self.tvEntityID)
        Entity.SetActive(self.tvEntityID, true)
        Entity.AddDialogueComponent(self.tvEntityID, "PLAYER_BEDROOM_TV", true, 10)

        print("Hello!")


        self.laptopEntityID = Entity.Create(9 * 32, 6 * 32)
        print("Laptop Entity ID: "..self.laptopEntityID)
        Entity.SetActive(self.laptopEntityID, true)
        Entity.AddDialogueComponent(self.laptopEntityID, "PLAYER_BEDROOM_LAPTOP", true, 0)
    end

    self.onDeactivate = function(self)
        Entity.Destroy(self.tvEntityID)
        Entity.Destroy(self.laptopEntityID)
    end

    self.update = function(self, dt)
    end
end
