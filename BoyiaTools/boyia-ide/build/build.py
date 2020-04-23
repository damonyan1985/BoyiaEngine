import os

plugin_dir = '../plugin'

build_extension = (
  f'cd {plugin_dir} &&'
  'npm run build &&'
  'vsce package'
)



def main():
  os.system(build_extension)

if __name__ == '__main__':
  main()