const UtilRegister = require('./registry/UtilRegister');
const UtilApiRegister = require('./registry/UtilApiRegister');
const KeyworkRegister = require('./registry/KeyworkRegister');

class CodeRegistry {
  static register() {
    CodeRegistry.keyword = new KeyworkRegister();
    CodeRegistry.registers = {};
    CodeRegistry.registers['U'] = new UtilRegister();
    CodeRegistry.registers['Util.'] = new UtilApiRegister();
  }
}

module.exports = CodeRegistry;