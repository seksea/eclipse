import discord
import json
import time

def loadDb() -> dict:
    s = ""
    with open("users.json", "r") as f:
        s = f.read()
    return json.loads(s)

def saveDb(db : dict):
    with open("users.json", "w") as f:
        s = f.write(json.dumps(users, indent=4))

def getHighestUid(db : dict) -> int:
    highest = 0
    for user in db["users"]:
        if db["users"][user]["uid"] > highest:
            highest = users[user]["uid"]
    return highest

client = discord.Client()

@client.event
async def on_ready():
    print(f"logged in as {client.user}")

@client.event
async def on_message(message : discord.Message):
    if message.content.startswith("."):
        if discord.utils.get(message.guild.roles, name="moderator") in message.author.roles:
            if message.content[1:].startswith("nuke"):
                await message.channel.purge()
            elif message.content[1:].startswith("purge"):
                await message.channel.purge(limit=int(message.content.split(" ")[1]))
            elif message.content[1:].startswith("addUser"):
                users = loadUsers()
                users[str(message.mentions[0].id)] = {
                    "uid" : getHighestUid(users) + 1,
                    "invitedBy" : "679310900951253043", #sekc
                    "joinTime" : int(time.time()),
                    "subEnd" : 0,
                    "banned": False,
                    "banReason": ""
                }
                saveUsers(users)

                embed=discord.Embed(title=f"Added user {message.mentions[0].name} to database.")
                embed.add_field(name="uid", value=users[str(message.mentions[0].id)]["uid"], inline=False)
                await message.reply(embed=embed)

            elif message.content[1:].startswith("addUser"):
                users = loadUsers()
                users[str(message.mentions[0].id)] = {
                    "uid" : getHighestUid(users) + 1,
                    "invitedBy" : "679310900951253043", #sekc
                    "joinTime" : int(time.time()),
                    "subEnd" : 0,
                    "banned": False,
                    "banReason": ""
                }
                saveUsers(users)

                embed=discord.Embed(title=f"Added user {message.mentions[0].name} to database.")
                embed.add_field(name="uid", value=users[str(message.mentions[0].id)]["uid"], inline=False)
                await message.reply(embed=embed)

        elif discord.utils.get(message.guild.roles, name="member") in message.author.roles:
            print("not mod")

client.run("OTA2OTM3MjYzMTczMjgzODYw.YYf5JQ.MFRu0tjqJDlMbl2A26obYnNSjZM")