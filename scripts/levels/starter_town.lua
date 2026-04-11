function init(self)
    self.updateCounter = 0

    self.onActivate = function(self)
        self.updateCounter = 0
    end

    self.onDeactivate = function(self)
        self.updateCounter = 0
    end

    self.update = function(self, dt)
        self.updateCounter = self.updateCounter + 1
        print(self.updateCounter)
    end
end