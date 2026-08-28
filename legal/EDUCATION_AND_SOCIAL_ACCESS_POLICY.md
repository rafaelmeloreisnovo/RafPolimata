# Education & Social Access Policy v1

This policy implements the fee-free noncommercial access grant for Original RAFAELIA Material while minimizing licensing-related personal-data collection.

## 1. Schools and educational use

For use permitted by `LicenseRef-RAFAELIA-RCNC-1.0`:

- no license fee is required for lawful noncommercial teaching, study or research;
- no RAFAELIA institutional registration is required merely to exercise that permission;
- no RAFAELIA student account/registration is required merely to exercise that permission;
- students should not be used as administrative/license contacts where a teacher, guardian, responsible adult or appropriate institutional channel can perform that role;
- a teacher/authorized adult may be the registered operational contact when registration is voluntarily needed for a service feature;
- where one telephone number for the responsible person/system is sufficient for the stated operational purpose, additional identifying data must not be collected merely for convenience.

This policy concerns RAFAELIA licensing/service administration only. It does **not** exempt a school, teacher, guardian, institution or data controller from records, safeguarding, education, tax, consumer, labor, privacy or other duties independently required by law.

## 2. Children and adolescents

Any processing of personal data involving children or adolescents must prioritize their best interests and comply with applicable privacy/data-protection requirements. Data collection must be purpose-specific, adequate, necessary, transparent and secure. Where a lawful service can operate through an adult/institutional contact without collecting a student's personal data, that lower-data path is preferred.

No public repository receipt should contain unnecessary student personal data.

## 3. Literacy-neutral community access

No literacy test, reading-level proof, formal schooling proof or socioeconomic certificate is required to exercise the fee-free noncommercial permissions.

This includes, without limitation, people who are illiterate or have limited literacy, autonomous collectors of recyclable/reusable/scrap materials, repair/reuse workers, community makers and other independent community users.

Accessibility may be provided through audio, icons, assisted communication, plain-language summaries or a trusted responsible person. Lack of literacy does not reduce the license permission.

## 4. No intermediary commercial capture

A commercial intermediary may not invoke a beneficiary's educational/community status to commercially resell, host, sublicense or monetize Covered Material without separate commercial authorization. The beneficiary's own permitted noncommercial use remains unaffected.

## 5. Minimal contact record

When a contact record is genuinely required for an optional operational service, use the smallest sufficient structure:

```yaml
contact_role: teacher|guardian|responsible_adult|institution_system_responsible|community_responsible
contact_channel:
  phone: TOKEN_VAZIO
purpose: TOKEN_VAZIO
necessity_reason: TOKEN_VAZIO
retention_rule: TOKEN_VAZIO
student_personal_data_collected: false
additional_fields:
  status: PROHIBITED_UNLESS_NECESSARY_AND_DOCUMENTED
```

A phone number is not mandatory where no contact is needed. If a different field is legally or operationally necessary, record the purpose and necessity rather than silently expanding collection.

## 6. Non-discrimination

Access administration must not discriminate on literacy, disability, economic condition, occupation or educational status. Necessary eligibility distinctions must be tied to the license scope (for example, commercial versus noncommercial use), not social stigma.
