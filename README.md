# PDB
ODBC 기반의 DB 연결 Library

### 개발년도
1. 2019년 10월 - 최초개발
2. 2021년 09월 - 부분수정

### 목적

Altibase DB 에 맞는 Application (DB 클라이언트) 에서의 이중화 구현


### 주요 동작

1. Application 요청에 따른, DB 연결 관리
2. DB 장애 및 에러 응답 시, Alternative DB 로 재시도 처리
3. DB 상태 주기적 확인 및 DB 연결 재시도
