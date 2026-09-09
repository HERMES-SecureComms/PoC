# HERMES Simulator

**Press. Authenticate. Communicate.**

Credential, PTT, 시간 만료, 철회, 장애를 재현하는 HERMES의 software-only reference simulator.

## 목적과 배경

HERMES는 Operator-bound Secure PTT 프로젝트다. 정상 무전기나 PTT 장비가
물리적으로 탈취되어도, 현재 사용하는 사람이 허용된 Credential을 가지고
있는지 확인해 무단 통신을 제한하는 구조를 연구한다.

Simulator v0.1은 실제 장비 없이 이 정책과 상태 전이를 PC에서 검증한다.
`GLOVE-001` 같은 ID는 예제 등록 정보를 조회하는 식별자다. `AUTH SUCCESS`는
“credential accepted”를 의미하며 실제 사용자 신원이나 암호학적 인증을 증명하지 않는다.

## 왜 Simulator가 필요한가

- NFC 리더나 ESP32 없이 인증·PTT·철회 동작을 반복 검증할 수 있다.
- 실제로 기다리지 않고 가상 시간을 진행해 timeout 경계를 확인한다.
- PTT를 누른 채 Credential을 잃는 경우처럼 이벤트 순서가 중요한 동작을 재현한다.
- 구조화된 이벤트와 JSON 시나리오를 실제 펌웨어 테스트의 비교 기준으로 사용할 수 있다.

## 전체 Architecture

```text
Interactive CLI / JSON Scenario
             |
             v
         Simulator ------------> VirtualClock (ms)
             |
             v
       State Machine ----------> Event History
             |
       immutable context
             v
        Policy Engine
             |
             v
       TX ALLOW / DENY + reason
```

CLI는 명령 파싱과 출력만 담당한다. 상태 전이와 TX 결정은 각각 독립된
`state_machine.py`, `policy.py`에서 처리한다. 실제 시간, NFC, GPIO, RF,
네트워크를 읽거나 제어하지 않는다.

## 주요 기능

- 등록 Credential 부착·분리, 미등록/비허용 Credential 거부
- 기본 1,500 ms grace period와 가상 시계
- PTT press/release와 TX 허용·거부 사유
- 장치 및 Credential 철회 시 활성 권한 즉시 무효화
- 장애, 복구, 재부팅 시 fail-closed 상태 전이
- 상태 조회와 timestamp가 있는 구조화된 이벤트
- 단계별 기대 상태 검증을 지원하는 JSON scenario runner
- pytest 단위·정책 조합·시나리오·실제 CLI subprocess 테스트

## 설치 방법

Python **3.12 이상**이 필요하다. 프로젝트 루트인 `hermes-simulator`에서 실행한다.

```bash
python -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements-dev.txt
```

Windows PowerShell에서는 활성화 명령을 `.venv\Scripts\Activate.ps1`로 바꾼다.
테스트 도구가 필요 없다면 `python -m pip install -e .`만 실행해도 된다.
실행 코드에는 외부 패키지 의존성이 없으며, 개발 의존성은 pytest다.

`src/` 레이아웃이므로 아래 `python -m` 명령은 위 설치 후 사용한다.
설치 없이 잠깐 실행하려면 POSIX shell에서 `PYTHONPATH=src python -m hermes_simulator.cli shell`도 가능하다.

## CLI 사용법과 명령어

```bash
python -m hermes_simulator.cli shell
# 설치된 console entry point도 동일하다.
hermes-simulator shell
```

| 명령 | 동작 |
|---|---|
| `credential attach GLOVE-001` | Credential 제시 및 명시적 재인증 |
| `credential detach` | 분리; 유효한 인증이면 grace 시작 |
| `credential revoke GLOVE-001` | 등록 Credential 철회; 현재 세션이면 즉시 무효화 |
| `ptt press` | PTT 누름; 현재 정책으로 TX 평가 |
| `ptt release` | PTT 해제; TX 비활성화 |
| `time advance 500` | 가상 시간 500 ms 전진; 만료와 TX 즉시 재평가 |
| `device revoke` | 장치 철회 및 현재 권한 무효화 |
| `device restore` | 장치 철회 해제; 이전 인증은 복구하지 않음 |
| `system fault` | 시스템 장애와 fail-closed 전환 |
| `system recover` | healthy 복귀; Credential 재인증 필요 |
| `system reboot` | healthy, PTT released, 인증 비활성화, TX deny |
| `status` | 상태·정확한 decision·시간·grace deadline 조회 |
| `events` | 이벤트 전체를 JSON lines로 출력 |
| `help` | 명령어 도움말 |
| `exit` / `quit` | 종료; EOF/Ctrl+C도 종료 |

음수·소수 시간이나 잘못된 명령은 오류로 처리한다. 명령 입력 오류가 발생해도
shell은 종료되지 않으며 그 명령이 기존 상태를 변경하지 않는다.

## 예제 session

기본 등록 정보는 `GLOVE-001` / `OPERATOR-001`, 장치는 `HPTT-001`이다.
아래는 주요 출력만 발췌한 예시다.

```text
> status
Auth State      : UNAUTHENTICATED
TX              : DENIED (DISABLED)

> credential attach GLOVE-001
[HERMES] CREDENTIAL DETECTED: GLOVE-001
[HERMES] AUTH SUCCESS
[HERMES] OPERATOR: OPERATOR-001

> ptt press
[HERMES] PTT PRESSED
[HERMES] TX ALLOWED

> ptt release
[HERMES] PTT RELEASED
[HERMES] TX DISABLED

> credential detach
[HERMES] CREDENTIAL LOST
[HERMES] ENTERING GRACE PERIOD
[HERMES] GRACE PERIOD: 1500 ms

> time advance 1000
[HERMES] TIME +1000 ms
[HERMES] AUTH STILL VALID IN GRACE PERIOD

> time advance 600
[HERMES] TIME +600 ms
[HERMES] AUTH EXPIRED
[HERMES] TX PERMISSION REVOKED

> ptt press
[HERMES] PTT PRESSED
[HERMES] TX DENIED
[HERMES] REASON: CREDENTIAL_EXPIRED
```

시간은 **명시적으로 진행한 만큼만** 흐른다. 터미널에서 10초 기다려도
`time advance`를 입력하지 않으면 인증은 만료되지 않는다.

## Configuration

```bash
python -m hermes_simulator.cli shell --config config.example.json
python -m hermes_simulator.cli run scenarios/stolen_radio.json --config config.example.json
```

`config.example.json`에서 `device_id`, `grace_period_ms`, 등록 `credentials`를
설정한다. Credential 필드는 `credential_id`, `operator_id`, `authorized`,
`revoked`이며, 목록을 빈 배열로 설정하면 등록 Credential이 없는 환경이 된다.
오타 필드, 중복 ID, 문자열로 전달한 boolean 등은 시작 시 거부한다.

1,500 ms는 **현재 hardware PoC의 인식 유예 동작을 모델링하는 simulation parameter**다.
실제 제품의 보안 session timeout을 정의한 값이 아니다. 0이면 분리 즉시 만료한다.

## Scenario 실행 방법

```bash
python -m hermes_simulator.cli run scenarios/normal_operation.json
python -m hermes_simulator.cli run scenarios/stolen_radio.json
python -m hermes_simulator.cli run scenarios/credential_loss_during_tx.json
python -m hermes_simulator.cli run scenarios/unknown_credential.json
python -m hermes_simulator.cli run scenarios/revoked_device.json
```

제공 시나리오는 모든 단계에 `expect`를 포함한다. 예를 들어 인증 없이 PTT를
누르면 `DENY_NO_CREDENTIAL`인지 검증한다.

```json
{
  "name": "no-credential",
  "steps": [
    {
      "action": "ptt_press",
      "expect": {"decision": "DENY_NO_CREDENTIAL", "tx_enabled": false}
    }
  ]
}
```

지원 action은 `attach_credential`, `detach_credential`, `revoke_credential`,
`advance_time`, `ptt_press`, `ptt_release`, `revoke_device`, `restore_device`,
`system_fault`, `system_recover`, `system_reboot`다. Credential action에는
`credential_id`, 시간 action에는 정수 `milliseconds`를 전달한다.

`expect`는 status의 필드를 부분 지정한다. 대표 필드는 `auth_state`, `decision`,
`ptt_state`, `tx_enabled`, `session_valid`, `device_revoked`, `system_healthy`,
`time_ms`, `grace_deadline_ms`, `credential_id`, `operator_id`, `device_id`다.
전체 파일을 먼저 검증하므로 잘못된 action/field가 있으면 어느 단계도 실행하지 않는다.

```text
Scenario: stolen-radio
Steps: 6
Passed: 6
Failed: 0
RESULT: PASS
```

종료 코드는 PASS `0`, 기대 상태 불일치/단계 실행 실패 `1`, 입력·파일 오류 `2`다.
기대 상태 불일치는 실제 실패로 집계하고 이후 단계도 실행한다.
`expect`를 생략한 사용자 시나리오도 실행할 수 있으나, 그 단계의 PASS는 명령
실행 및 기본 invariant 통과만 뜻한다. 원하는 동작의 입증에는 명시적 `expect`가 필요하다.

## pytest 실행 방법

```bash
pytest
```

테스트는 `FakeClock`을 사용하며 실제 시간 대기를 하지 않는다. TC-SIM-001~012의
대응 테스트, 추가 경계 조건, 실행 절차는 [TEST_PLAN.md](docs/TEST_PLAN.md)에 있다.
실행 환경과 실제 검증 결과는 [VALIDATION.md](docs/VALIDATION.md)에 기록한다.

## Fail-Closed 정책

```text
TX ALLOW = system healthy
       AND device not revoked
       AND credential authorized and not revoked
       AND valid authentication/session
       AND PTT pressed
```

그 외에는 항상 DENY다. 사유의 우선순위는 시스템 장애 → 장치 철회 →
Credential 철회/비허용 → 인증 무효/만료 → PTT 해제다. 정상 인증이어도 PTT가
해제되면 `DENY_PTT_RELEASED`이고 TX는 꺼진다. 알 수 없거나 모순된 상태도 거부한다.

유예 만료는 `now_ms >= grace_deadline_ms`일 때 발생한다. PTT를 계속 누르고
있어도 시간 전진 명령에서 TX가 꺼지며, 다음 PTT 입력을 기다리지 않는다.
같은 분리 명령을 반복해도 deadline은 늘어나지 않는다. 미등록 Credential로
교체하면 이전 세션의 유예를 물려받지 않고 즉시 거부한다.

PTT는 level 방식이다. 계속 누르는 중 정상 Credential을 새로 제시하면
재인증 직후 TX가 다시 허용될 수 있다. 재누름을 강제하는 interlock은 v0.1에 없다.

`device restore`와 `system recover` 후에는 `credential attach ID`로 다시 인증해야 한다.
재부팅은 부착 정보·세션·PTT를 초기화하지만, 같은 프로세스의 철회 기록과
이벤트·가상 시간은 유지한다. 이미 철회된 장치는 재부팅해도 `REVOKED`다.

## 현재 한계와 보안 표현

- 실제 NFC/사용자 신원/암호학적 challenge-response를 검증하지 않는다.
- 실제 hardware PoC는 UID whitelist 기반이다. UID는 복제 가능한 식별자이며
  보안 인증 방식이 아니다. Simulator의 ID 등록 확인도 이를 강화하지 않는다.
- **공격자가 Credential 자체를 장비와 함께 탈취하면 v0.1은 원 사용자와 공격자를 구분하지 못한다.**
- 복제된 ID나 도난 Credential의 사용을 막는 liveness/추가 요소는 없다.
- grace 안에서는 분리 후에도 권한이 유지된다. 이것은 의도한 노출 구간이다.
- `device restore`, `system recover` 등은 시뮬레이션 제어 명령이다.
  **production 환경에서 장치 restore를 단순 로컬 명령으로 허용해서는 안 된다.**
- 이벤트와 철회는 메모리에만 있다. shell 종료/새 프로세스 시작 시 사라진다.
- OS, 입력 configuration, 로컬 사용자는 테스트 제어자로 신뢰한다.
- 물리적 GPIO 안전, RF 동작, 실제 지연, 동시성·실시간 스케줄링은 검증하지 않는다.

위협별 범위는 [THREAT_MODEL.md](docs/THREAT_MODEL.md)에 명시했다.

## 실제 HERMES PoC와의 관계

```text
Simulator (Software Reference Model):
Credential → State Machine → Policy Engine → TX Decision

Firmware (ESP32-S3 + PN532 implementation):
PN532 → ESP32 Policy Logic → TX_GATE
```

두 시스템은 동일한 기본 TX 허용 정책을 목표로 한다. 다만 simulator는 명시적
detach 시점을 알고 그때 grace를 시작하며, hardware PoC는 마지막 정상 NFC
읽기를 기준으로 누락을 판단한다. 실제 polling/debounce/부팅 핀 상태는 모델에 없다.
장치 철회·복구는 simulator에서 검증하는 정책이며, 현재 firmware에 구현되어
있다는 뜻이 아니다. 따라서 자동적인 동등성이나 실물 검증 완료를 주장하지 않는다.

향후 동일한 사건 순서와 시간 기준을 정의해 firmware trace를 비교한다.
자세한 비교 지점은 [ARCHITECTURE.md](docs/ARCHITECTURE.md)에 있다.

## 파일 구성

```text
hermes-simulator/
├── README.md
├── pyproject.toml
├── requirements-dev.txt
├── config.example.json
├── src/hermes_simulator/
│   ├── __init__.py
│   ├── cli.py
│   ├── models.py
│   ├── state_machine.py
│   ├── policy.py
│   ├── clock.py
│   ├── events.py
│   └── simulator.py
├── tests/
│   ├── conftest.py
│   ├── test_authentication.py
│   ├── test_ptt.py
│   ├── test_revocation.py
│   ├── test_fail_closed.py
│   ├── test_scenarios.py
│   └── test_cli.py
├── scenarios/
│   ├── normal_operation.json
│   ├── stolen_radio.json
│   ├── unknown_credential.json
│   ├── credential_loss_during_tx.json
│   └── revoked_device.json
└── docs/
    ├── ARCHITECTURE.md
    ├── STATE_MACHINE.md
    ├── THREAT_MODEL.md
    ├── TEST_PLAN.md
    └── VALIDATION.md
```

## Roadmap / 남은 TODO

- v0.1 후속: hardware trace와 reference state 비교 형식 및 실측 기준 정의
- v0.2 연구: Secure NFC, challenge-response, secure element의 신뢰 가정을 정책에 반영
- 세션 재인증과 PTT 재누름 요구 여부를 제품 정책으로 결정
- production 복구/철회 권한과 재시작 후 철회 유지 정책 설계

GUI, web dashboard, backend, database, cloud, 사용자 계정, 실제 NFC/RF/radio
PTT 제어, SDR, Bluetooth, Meshtastic 송신, OpenMANET/ATAK networking,
실제 암호 인증 및 production deployment는 이번 구현 범위에 없다.

