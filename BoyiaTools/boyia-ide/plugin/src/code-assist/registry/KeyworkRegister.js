const BaseRegister = require('./BaseRegister');

class KeyworkRegister extends BaseRegister {
  exec(text, config) {
    switch (text) {
      case 'c':
        return this.getAssist(['class']);
      case 'f':
        return this.getAssist(['fun', 'false']);
      case 'p':
        return this.getAssist(['prop']);
      case 'v':
        return this.getAssist(['var']);
      case 'd':
        return this.getAssist(['do']);
      case 'w':
        return this.getAssist(['while']);
      case 'e':
        return this.getAssist(['extends']);
      case 'r':
        return this.getAssist(['return']);
      case 't':
        return this.getAssist(['true']);
    }

    return this.getAssist([]);
  }

  isKeyword() {
    return true;
  }
}

module.exports = KeyworkRegister;