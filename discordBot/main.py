import discord
import json
import time
import random
import string
from datetime import datetime

def loadDb() -> dict:
    s = ""
    with open("database.json", "r") as f:
        s = f.read()
    return json.loads(s)

def saveDb(db : dict):
    with open("database.json", "w") as f:
        s = f.write(json.dumps(db, indent=4))

def getHighestUid(users : dict) -> int:
    highest = 0
    for user in users:
        if users[user]["uid"] > highest:
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
            if message.content[1:].lower().startswith("nuke"):
                await message.channel.purge()

            elif message.content[1:].lower().startswith("purge"):
                await message.channel.purge(limit=int(message.content.split(" ")[1]))

            elif message.content[1:].lower().startswith("useradd"):
                db = loadDb()
                db["users"][str(message.mentions[0].id)] = {
                    "uid" : getHighestUid(db["users"]) + 1,
                    "invitedBy" : str("679310900951253043"), #sekc
                    "joinTime" : int(time.time()),
                    "subEnd" : 0,
                    "banned": False,
                    "banReason": ""
                }
                saveDb(db)

                embed=discord.Embed(title=f"added user {message.mentions[0].name} to database")
                embed.add_field(name="uid", value=db["users"][str(message.mentions[0].id)]["uid"], inline=False)
                await message.reply(embed=embed)

            elif message.content[1:].lower().startswith("geninvite"):
                db = loadDb()
                invite = "ecl"
                for i in range(32):
                    invite += random.choice(string.ascii_lowercase + string.ascii_uppercase + string.digits + "!.")
                db["invites"][invite] = {
                    "owner": message.author.id #sekc
                }
                saveDb(db)

                embed=discord.Embed(title=f"Generated invite")
                embed.add_field(name="invite", value=invite, inline=False)
                await message.reply(embed=embed)

        elif discord.utils.get(message.guild.roles, name="member") in message.author.roles:
            print("not mod")

        else:
            if message.content[1:].lower().startswith("useinvite"):
                db = loadDb()
                if message.content.split(" ")[1] in db["invites"]:
                    db["invites"].remove(message.content.split(" ")[1])
                    db["users"][str(message.author.id)] = {
                        "uid" : getHighestUid(db["users"]) + 1,
                        "invitedBy" : db["invites"][message.content.split(" ")[1]]["owner"],
                        "joinTime" : int(time.time()),
                        "subEnd" : 0,
                        "banned": False,
                        "banReason": ""
                    }
                    saveDb(db)
                    embed=discord.Embed(title=f"invited {message.author.name} to database")
                    embed.add_field(name="uid", value=db["users"][str(message.author.id)]["uid"], inline=False)
                    await message.reply(embed=embed)
                    return

                embed=discord.Embed(title=f"invite invalid")
                await message.reply(embed=embed)
        
        if message.content[1:].lower().startswith("userinfo"):
            db = loadDb()
            if str(message.mentions[0].id) in db["users"]:
                print(int(db["users"][str(message.mentions[0].id)]["invitedBy"]))
                print(message.guild.get_member(int(db["users"][str(message.mentions[0].id)]["invitedBy"])))
                embed=discord.Embed(title=f"{message.mentions[0].name} info")
                embed.add_field(name="uid", value=db["users"][str(message.mentions[0].id)]["uid"], inline=False)
                embed.add_field(name="invited by", value=message.guild.get_member(int(db["users"][str(message.mentions[0].id)]["invitedBy"])), inline=False)
                embed.add_field(name="join date", value=datetime.utcfromtimestamp(db["users"][str(message.mentions[0].id)]["joinTime"]).strftime('%d %m %Y'), inline=False)
                await message.reply(embed=embed)
                return

            embed=discord.Embed(title=f"user not a member")
            await message.reply(embed=embed)

client.run("OTA2OTM3MjYzMTczMjgzODYw.YYf5JQ.MFRu0tjqJDlMbl2A26obYnNSjZM")