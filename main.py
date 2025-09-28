import os
import asyncio
import aiohttp
import aiofiles

RIR_DICT = {
    "ARIN": {
        "region": "ARIN",
        "base_url": "https://ftp.arin.net/pub/stats/arin/",
        "file": "delegated-arin-extended-latest",
    },
    "RIPE NCC": {
        "region": "RIPE NCC",
        "base_url": "https://ftp.ripe.net/pub/stats/ripencc/",
        "file": "delegated-ripencc-latest",
    },
    "APNIC": {
        "region": "APNIC",
        "base_url": "https://ftp.apnic.net/pub/stats/apnic/",
        "file": "delegated-apnic-latest",
    },
    "LACNIC": {
        "region": "LACNIC",
        "base_url": "https://ftp.lacnic.net/pub/stats/lacnic/",
        "file": "delegated-lacnic-latest",
    },
    "AFRINIC": {
        "region": "AFRINIC",
        "base_url": "https://ftp.afrinic.net/pub/stats/afrinic/",
        "file": "delegated-afrinic-latest",
    },
}


async def fetch(session, url):
    async with session.get(url) as response:
        response.raise_for_status()
        return await response.read()


async def fetch_rir(session, region):
    base_url = region["base_url"]
    filename = region["file"]
    region_name = region["region"]

    try:
        print(f"Checking updates for {region_name} ...")

        # md5 の取得
        md5_url = f"{base_url}{filename}.md5"
        remote_md5 = await fetch(session, md5_url)

        local_md5_path = f"data/{filename}.md5"
        local_file_path = f"data/{filename}.psv"

        if os.path.exists(local_md5_path):
            async with aiofiles.open(local_md5_path, "rb") as fp:
                if remote_md5 == await fp.read():
                    print(f"No update for {filename}, skipping download.")
                    return

        # データ本体を取得
        data_url = f"{base_url}{filename}"
        print(f"Fetching {filename} ...")
        data = await fetch(session, data_url)

        # データの保存
        async with aiofiles.open(local_file_path, "wb") as fp:
            await fp.write(data)

        # md5 の保存
        async with aiofiles.open(local_md5_path, "wb") as fp:
            await fp.write(remote_md5)

        print(f"Updated {filename}")

    except Exception as e:
        print(f"Error accessing {base_url}: {str(e)}")


async def main():
    os.makedirs("data", exist_ok=True)

    timeout = aiohttp.ClientTimeout(total=60)
    async with aiohttp.ClientSession(timeout=timeout) as session:
        tasks = [fetch_rir(session, stats) for stats in RIR_DICT.values()]
        await asyncio.gather(*tasks)

if __name__ == "__main__":
    asyncio.run(main())
    print("Fetching RIR data completed.")
