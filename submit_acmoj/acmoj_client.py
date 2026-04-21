#!/usr/bin/env python3
import argparse
import json
import os
from datetime import datetime
from typing import Any, Dict, Optional

import requests


class ACMOJClient:
    def __init__(self, access_token: str):
        self.api_base = 'https://acm.sjtu.edu.cn/OnlineJudge/api/v1'
        self.headers = {
            'Authorization': f'Bearer {access_token}',
            'Content-Type': 'application/x-www-form-urlencoded',
            'User-Agent': 'ACMOJ-Python-Client/2.2',
        }
        self.submission_log_file = '/workspace/submission_ids.log'

    def _make_request(self, method: str, endpoint: str, data: Optional[Dict[str, Any]] = None,
                      params: Optional[Dict[str, Any]] = None) -> Optional[Dict[str, Any]]:
        url = f'{self.api_base}{endpoint}'
        try:
            if method.upper() == 'GET':
                resp = requests.get(url, headers=self.headers, params=params, timeout=10,
                                    proxies={'https': None, 'http': None})
            elif method.upper() == 'POST':
                resp = requests.post(url, headers=self.headers, data=data, timeout=10,
                                     proxies={'https': None, 'http': None})
            else:
                print(f'Unsupported HTTP method: {method}')
                return None

            if resp.status_code == 204:
                return {'status': 'success', 'message': 'Operation successful'}

            resp.raise_for_status()
            return resp.json() if resp.content else {'status': 'success'}
        except requests.exceptions.RequestException as e:
            print(f'API Request failed: {e}')
            try:
                print(f'Response text: {resp.text}')
            except Exception:
                pass
            return None

    def _save_submission_id(self, submission_id: int) -> None:
        try:
            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            entry = {'timestamp': timestamp, 'submission_id': submission_id}
            with open(self.submission_log_file, 'a', encoding='utf-8') as f:
                f.write(json.dumps(entry) + '\n')
            print(f'Submission ID {submission_id} saved to {self.submission_log_file}')
        except Exception as e:
            print(f'Warning: Failed to save submission ID: {e}')

    def submit_code(self, problem_id: int, language: str, code_text: str) -> Optional[Dict[str, Any]]:
        data = {'language': language, 'code': code_text}
        res = self._make_request('POST', f'/problem/{problem_id}/submit', data=data)
        if res and 'id' in res:
            self._save_submission_id(res['id'])
        return res

    def get_submission_detail(self, submission_id: int) -> Optional[Dict[str, Any]]:
        return self._make_request('GET', f'/submission/{submission_id}')

    def abort_submission(self, submission_id: int) -> Optional[Dict[str, Any]]:
        return self._make_request('POST', f'/submission/{submission_id}/abort')


def main() -> None:
    parser = argparse.ArgumentParser(description='ACMOJ API Command Line Client')
    parser.add_argument('--token', help='ACMOJ Access Token', default=os.environ.get('ACMOJ_TOKEN'))

    subparsers = parser.add_subparsers(dest='command', required=True)

    submit_parser = subparsers.add_parser('submit', help='Submit a source file')
    submit_parser.add_argument('--problem-id', type=int, required=True)
    submit_parser.add_argument('--language', type=str, required=True)
    submit_parser.add_argument('--code-file', type=str, required=True)

    status_parser = subparsers.add_parser('status', help='Check submission status')
    status_parser.add_argument('--submission-id', type=int, required=True)

    abort_parser = subparsers.add_parser('abort', help='Abort submission evaluation')
    abort_parser.add_argument('--submission-id', type=int, required=True)

    args = parser.parse_args()

    if not args.token:
        print('Error: Access token not provided. Use --token or set ACMOJ_TOKEN')
        return

    client = ACMOJClient(args.token)

    if args.command == 'submit':
        try:
            with open(args.code_file, 'r', encoding='utf-8') as f:
                code_text = f.read()
        except FileNotFoundError:
            print(f'Error: Code file not found at {args.code_file}')
            return
        except Exception as e:
            print(f'Error: Failed to read code file: {e}')
            return
        result = client.submit_code(args.problem_id, args.language, code_text)
    elif args.command == 'status':
        result = client.get_submission_detail(args.submission_id)
    elif args.command == 'abort':
        result = client.abort_submission(args.submission_id)
    else:
        print('Unknown command')
        return

    if result:
        print(json.dumps(result))
    else:
        raise SystemExit(1)


if __name__ == '__main__':
    main()

