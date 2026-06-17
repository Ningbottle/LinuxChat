import os

dirs = [
    r'd:\ChatBox\LinuxChat\server\include',
    r'd:\ChatBox\LinuxChat\server\src',
    r'd:\ChatBox\LinuxChat\server\third_party\nlohmann',
    r'd:\ChatBox\LinuxChat\client\include',
    r'd:\ChatBox\LinuxChat\client\src',
    r'd:\ChatBox\LinuxChat\client\resources',
    r'd:\ChatBox\LinuxChat\tests',
    r'd:\ChatBox\LinuxChat\docs',
]

for d in dirs:
    os.makedirs(d, exist_ok=True)
    print(f'Created: {d}')

print('All directories created.')
