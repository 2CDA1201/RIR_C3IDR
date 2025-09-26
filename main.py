import os
import requests

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

def retr_https(region):
    base_url = region["base_url"]
    filename = region["file"]
    region_name = region["region"]

    try:
        print(f"Checking {region_name} ...")

        # md5 の取得
        md5_url = f"{base_url}{filename}.md5"
        r = requests.get(md5_url, timeout=30)
        r.raise_for_status()
        remote_md5 = r.content

        local_md5_path = f"data/{filename}.md5"
        local_file_path = f"data/{filename}.psv"

        if os.path.exists(local_md5_path):
            with open(local_md5_path, "rb") as fp:
                if remote_md5 == fp.read():
                    print(f"No update for {filename}, skipping download.")
                    return

        # データ本体を取得
        data_url = f"{base_url}{filename}"
        print(f"Retrieving {filename} ...")
        r = requests.get(data_url, timeout=60)
        r.raise_for_status()
        with open(local_file_path, "wb") as fp:
            fp.write(r.content)

        # md5 の保存
        with open(local_md5_path, "wb") as fp:
            fp.write(remote_md5)

        print(f"Updated {filename}")

    except Exception as e:
        print(f"Error accessing {base_url}: {str(e)}")

def main():
    os.makedirs("data", exist_ok=True)
    for stats in RIR_DICT.values():
        retr_https(stats)

if __name__ == "__main__":
    main()
