const KeyworkRegister = require('./registry/KeyworkRegister');

class CodeRegistry {
  static register() {
    CodeRegistry.keyword = new KeyworkRegister();
    CodeRegistry.registers = {};
  }
}

module.exports = CodeRegistry;
